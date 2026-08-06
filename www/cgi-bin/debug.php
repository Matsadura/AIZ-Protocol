<?php
/**
 * debug.php — CGI/PHP diagnostic page.
 * Dumps query parameters, incoming request headers, and all CGI/server
 * environment variables provided by the web server.
 */
function h(string $s): string
{
    return htmlspecialchars($s, ENT_QUOTES, 'UTF-8');
}

$headers = function_exists('getallheaders') ? getallheaders() : [];
$query   = $_GET;
$server  = $_SERVER;

// Sort environment variables alphabetically by key for easier scanning
ksort($server);

header('Content-Type: text/html; charset=utf-8');
?>
<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="utf-8">
<title>CGI Environment Test</title>
<link rel="stylesheet" href="/styles/common.css">
<link rel="stylesheet" href="/styles/debug.css">
</head>
<body class="page">
<div class="debug-widget">
    <h1 class="debug-widget__title">Test CGI environment variables</h1>

    <section class="debug-widget__section">
        <h2 class="debug-widget__section-title">Query values</h2>
        <?php if (!$query): ?>
            <p class="debug-widget__empty">No query parameters.</p>
        <?php else: ?>
            <table class="debug-widget__table">
                <?php foreach ($query as $key => $value): ?>
                <tr class="debug-widget__row">
                    <th class="debug-widget__key"><?= h((string)$key) ?></th>
                    <td class="debug-widget__value"><?= h(is_scalar($value) ? (string)$value : print_r($value, true)) ?></td>
                </tr>
                <?php endforeach; ?>
            </table>
        <?php endif; ?>
    </section>

    <section class="debug-widget__section">
        <h2 class="debug-widget__section-title">Headers</h2>
        <?php if (!$headers): ?>
            <p class="debug-widget__empty">No headers received.</p>
        <?php else: ?>
            <table class="debug-widget__table">
                <?php foreach ($headers as $key => $value): ?>
                <tr class="debug-widget__row">
                    <th class="debug-widget__key"><?= h((string)$key) ?></th>
                    <td class="debug-widget__value"><?= h((string)$value) ?></td>
                </tr>
                <?php endforeach; ?>
            </table>
        <?php endif; ?>
    </section>

    <section class="debug-widget__section">
        <h2 class="debug-widget__section-title">Server &amp; CGI Environment ($_SERVER)</h2>
        <?php if (!$server): ?>
            <p class="debug-widget__empty">No server environment variables found.</p>
        <?php else: ?>
            <table class="debug-widget__table">
                <?php foreach ($server as $key => $value): ?>
                <tr class="debug-widget__row">
                    <th class="debug-widget__key"><?= h((string)$key) ?></th>
                    <td class="debug-widget__value"><?= h(is_scalar($value) ? (string)$value : print_r($value, true)) ?></td>
                </tr>
                <?php endforeach; ?>
            </table>
        <?php endif; ?>
    </section>
</div>
</body>
</html>


