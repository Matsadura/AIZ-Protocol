<?php
/**
 * watch.php — renders a single video, chosen via ?v=<filename>.
 * Only files that actually exist inside $videoDir, with an allowed
 * video extension, are ever played (basename() + extension check
 * blocks path traversal like ?v=../../etc/passwd).
 */

$videoDir = "/home/aljbari/goinfre";
$allowedExt = ['mp4', 'webm', 'ogg', 'ogv'];

function h(string $s): string
{
    return htmlspecialchars($s, ENT_QUOTES, 'UTF-8');
}

$file = basename((string)($_GET['v'] ?? ''));
$ext  = strtolower(pathinfo($file, PATHINFO_EXTENSION));
$path = $videoDir . '/' . $file;

$valid = $file !== ''
    && in_array($ext, $allowedExt, true)
    && is_file($path);

header('Content-Type: text/html; charset=utf-8');
?>
<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="utf-8">
<title><?= $valid ? h(pathinfo($file, PATHINFO_FILENAME)) : 'Video not found' ?></title>
<link rel="stylesheet" href="/styles/common.css">
<link rel="stylesheet" href="/styles/watch.css">
</head>
<body class="page">

<div class="watch-widget">
    <a class="watch-widget__back" href="watch_index.php">&larr; Back to videos</a>

    <?php if ($valid): ?>
        <h1 class="watch-widget__title"><?= h(pathinfo($file, PATHINFO_FILENAME)) ?></h1>
        <video class="watch-widget__video" controls autoplay>
            <source src="/uploads/<?= h(rawurlencode($file)) ?>">
        </video>
    <?php else: ?>
        <p class="watch-widget__empty">That video couldn't be found.</p>
    <?php endif; ?>
</div>

</body>
</html>
