#!/usr/bin/php-cgi
<?php
/**
 * chat.php — single-file CGI chat app
 * ------------------------------------------------------------------
 * Storage : plain text files (messages.txt, clients.txt), created
 *           automatically next to this file. No DB drivers needed —
 *           each is just append-only lines, guarded with flock() so
 *           concurrent requests don't interleave writes.
 * Identity: a random ID is minted server-side the first time a browser
 *           tab shows up. That ID is handed to the browser, which
 *           stashes it in window.sessionStorage (tab-scoped, cleared
 *           when the tab closes) and from then on the id also rides
 *           along in the URL as ?client_id=... so every GET (including
 *           a plain page refresh) is self-contained.
 * Flow    : GET  (no id)  -> mint id + name, send a tiny bootstrap page
 *                             that stores it in sessionStorage and
 *                             redirects (client-side) to ?client_id=...
 *           GET  (id)     -> render message log + form
 *           POST (id+msg) -> insert message, then 302 redirect back to
 *                             GET ?client_id=... so a refresh only ever
 *                             re-issues a GET, never resends the message
 *                             (Post/Redirect/Get pattern)
 * ------------------------------------------------------------------
 * Deploy as a real CGI script (e.g. Apache + mod_cgi / php-cgi):
 *   chmod +x chat.php
 *   AddHandler cgi-script .php   (inside a <Directory> that allows it)
 *   Options +ExecCGI
 * Or just run it under any normal PHP+Apache/Nginx setup — the code
 * below works the same either way, it only *behaves* like a CGI
 * script (stateless, one full page per request).
 */

declare(strict_types=1);

// ---------------------------------------------------------------
// Storage
// ---------------------------------------------------------------

$clientsFile  = __DIR__ . '/clients.txt';  // one client per line: id\tname\tcreated_at
$messagesFile = __DIR__ . '/messages.txt'; // one message per line, JSON-encoded

function ensure_storage_files(string $clientsFile, string $messagesFile): void
{
    foreach ([$clientsFile, $messagesFile] as $file) {
        if (!file_exists($file)) {
            touch($file);
        }
    }
}

// small helper: append a line to a file under an exclusive lock so
// two simultaneous requests can't interleave/corrupt each other's writes
function append_line(string $file, string $line): void
{
    $fh = fopen($file, 'a');
    if ($fh === false) {
        return;
    }
    flock($fh, LOCK_EX);
    fwrite($fh, $line . "\n");
    flock($fh, LOCK_UN);
    fclose($fh);
}

// ---------------------------------------------------------------
// Client identity: mint a fresh id + a friendly name (Sam, Alex...)
// ---------------------------------------------------------------

function generate_client_id(): string
{
    return bin2hex(random_bytes(16)); // 32 hex chars, plenty unique
}

function generate_name(): string
{
    $first = [
        'Sam', 'Alex', 'Jordan', 'Taylor', 'Morgan', 'Casey', 'Riley',
        'Jamie', 'Drew', 'Skyler', 'Avery', 'Quinn', 'Reese', 'Rowan',
        'Harper', 'Finley', 'Emerson', 'Blake', 'Charlie', 'Dana',
    ];
    // pick a random name, then append a short random number so
    // repeated names ("Sam", "Sam") stay visually distinguishable
    return $first[array_rand($first)] . random_int(10, 99);
}

function register_new_client(string $clientsFile): array
{
    $id = generate_client_id();
    $name = generate_name();
    // tab-separated: id and name never contain tabs, so no escaping needed
    append_line($clientsFile, implode("\t", [$id, $name, time()]));
    return ['id' => $id, 'name' => $name];
}

function load_client_names(string $clientsFile): array
{
    $map = [];
    $lines = file($clientsFile, FILE_IGNORE_NEW_LINES | FILE_SKIP_EMPTY_LINES) ?: [];
    foreach ($lines as $line) {
        [$id, $name] = array_pad(explode("\t", $line), 2, null);
        if ($id !== null && $name !== null) {
            $map[$id] = $name;
        }
    }
    return $map;
}

function get_client_name(string $clientsFile, string $id): ?string
{
    return load_client_names($clientsFile)[$id] ?? null;
}

// ---------------------------------------------------------------
// Messages
// ---------------------------------------------------------------

function insert_message(string $messagesFile, string $clientId, string $body): void
{
    $body = trim($body);
    if ($body === '') {
        return;
    }
    // keep messages reasonably sized
    $body = mb_substr($body, 0, 2000);
    // JSON-encode each line so newlines/tabs/quotes in the message
    // body can never break the one-line-per-message file format
    $record = json_encode([
        'client_id'  => $clientId,
        'body'       => $body,
        'created_at' => time(),
    ]);
    append_line($messagesFile, $record);
}

function fetch_messages(string $messagesFile, string $clientsFile, int $limit = 200): array
{
    $lines = file($messagesFile, FILE_IGNORE_NEW_LINES | FILE_SKIP_EMPTY_LINES) ?: [];
    $lines = array_slice($lines, -$limit); // most recent $limit lines, oldest first
    $names = load_client_names($clientsFile);

    $rows = [];
    foreach ($lines as $line) {
        $row = json_decode($line, true);
        if (!is_array($row)) {
            continue; // skip any malformed/corrupted line
        }
        $row['name'] = $names[$row['client_id']] ?? 'unknown';
        $rows[] = $row;
    }
    return $rows;
}

// ---------------------------------------------------------------
// Small helpers
// ---------------------------------------------------------------

function self_url(): string
{
    $scheme = (!empty($_SERVER['HTTPS']) && $_SERVER['HTTPS'] !== 'off') ? 'https' : 'http';
    $host = $_SERVER['HTTP_HOST'] ?? 'localhost';
    $path = $_SERVER['SCRIPT_NAME'] ?? $_SERVER['PHP_SELF'] ?? '/chat.php';
    return $scheme . '://' . $host . $path;
}

function h(string $s): string
{
    return htmlspecialchars($s, ENT_QUOTES, 'UTF-8');
}

// ---------------------------------------------------------------
// Main
// ---------------------------------------------------------------

ensure_storage_files($clientsFile, $messagesFile);
$method = $_SERVER['REQUEST_METHOD'] ?? 'GET';

// ---- POST: a new message is being sent ----
if ($method === 'POST') {
    $clientId = trim((string)($_POST['client_id'] ?? ''));
    $message  = (string)($_POST['message'] ?? '');

    if ($clientId !== '' && get_client_name($clientsFile, $clientId) !== null) {
        insert_message($messagesFile, $clientId, $message);
    }

    // Post/Redirect/Get: send the browser back to a plain GET (with the
    // id in the query string) so a manual page refresh only ever
    // re-issues a GET and never resends the message.
    header('Location: ' . self_url() . '?client_id=' . urlencode($clientId), true, 302);
    echo "Redirecting...\n";
    exit;
}

// ---- GET: no client id yet -> mint one and hand it to the browser ----
$clientId = trim((string)($_GET['client_id'] ?? ''));
$clientName = $clientId !== '' ? get_client_name($clientsFile, $clientId) : null;

if ($clientId === '' || $clientName === null) {
    $client = register_new_client($clientsFile);
    $bootstrapUrl = self_url() . '?client_id=' . urlencode($client['id']);
    header('Content-Type: text/html; charset=utf-8');
    ?>
<!DOCTYPE html>
<html>
<head><meta charset="utf-8"><title>Joining chat…</title></head>
<body>
<p>Joining chat as <?= h($client['name']) ?>…</p>
<script>
    // Tab-scoped identity: cleared automatically when the tab closes.
    sessionStorage.setItem('chat_client_id', <?= json_encode($client['id']) ?>);
    window.location.replace(<?= json_encode($bootstrapUrl) ?>);
</script>
</body>
</html>
    <?php
    exit;
}

// ---- GET with a valid client id -> render the chat page ----
$messages = fetch_messages($messagesFile, $clientsFile);
header('Content-Type: text/html; charset=utf-8');
?>
<!DOCTYPE html>
<html>
<head>
<meta charset="utf-8">
<title>CGI Chat</title>
<link rel="stylesheet" href="/styles/common.css">
<link rel="stylesheet" href="/styles/chat.css">
</head>
<body class="page">

<div class="chat-widget">
    <h1 class="chat-widget__title">CGI Chat</h1>
    <div class="chat-widget__meta">You are <strong><?= h($clientName) ?></strong></div>

    <div class="chat-widget__log" id="log">
    <?php if (!$messages): ?>
        <p class="chat-widget__empty">No messages yet — say hello!</p>
    <?php else: foreach ($messages as $m): ?>
        <div class="message<?= $m['client_id'] === $clientId ? ' message--mine' : '' ?>">
            <span class="message__name"><?= h($m['name'] ?? 'unknown') ?></span>
            <span class="message__time"><?= h(date('H:i:s', (int)$m['created_at'])) ?></span>
            <div class="message__body"><?= nl2br(h($m['body'])) ?></div>
        </div>
    <?php endforeach; endif; ?>
    </div>

    <!-- "form in the middle": the send box sits between the message log
         above and nothing below — the focal point of the page. -->
    <form class="chat-widget__form" id="send" method="post" action="<?= h(self_url()) ?>">
        <input type="hidden" name="client_id" value="<?= h($clientId) ?>">
        <input class="chat-widget__input" type="text" name="message" placeholder="Type a message…" autocomplete="off" autofocus required>
        <button class="chat-widget__button" type="submit">Send</button>
    </form>
</div>

<script>
    // Make sure this tab's sessionStorage always matches the id the
    // page was rendered with, and keep the URL carrying it too, so
    // reloading / re-visiting the bare script URL doesn't mint a
    // brand-new identity as long as the tab is still open.
    var CLIENT_ID = <?= json_encode($clientId) ?>;
    sessionStorage.setItem('chat_client_id', CLIENT_ID);

    (function () {
        var params = new URLSearchParams(window.location.search);
        if (!params.get('client_id')) {
            var stored = sessionStorage.getItem('chat_client_id');
            if (stored) {
                params.set('client_id', stored);
                window.location.replace(window.location.pathname + '?' + params.toString());
            }
        }
    })();

    // keep the log scrolled to the newest message
    var log = document.getElementById('log');
    log.scrollTop = log.scrollHeight;
</script>

</body>
</html>
