<?php
/**
 * debug.php — quick CGI/PHP diagnostic page.
 * Dumps the query string and all incoming request headers so you can
 * check what the server is actually receiving.
 */

function h(string $s): string
{
    return htmlspecialchars($s, ENT_QUOTES, 'UTF-8');
}

$headers = getallheaders() ?: [];
$query   = $_GET;

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
                    <td class="debug-widget__value"><?= h((string)$value) ?></td>
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
</div>

</body>
</html>
