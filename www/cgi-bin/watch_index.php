<?php
/**
 * videos.php — lists every video file in $videoDir as a simple gallery.
 * Clicking a video goes to watch.php?v=<filename>, which just renders
 * a <video> tag for that one file (like a YouTube watch page).
 */
 
$videoDir = "/home/aljbari/goinfre";
$allowedExt = ['mp4', 'webm', 'ogg', 'ogv'];
 
function h(string $s): string
{
    return htmlspecialchars($s, ENT_QUOTES, 'UTF-8');
}
 
$files = [];
if (is_dir($videoDir)) {
    foreach (scandir($videoDir) as $entry) {
        $ext = strtolower(pathinfo($entry, PATHINFO_EXTENSION));
        if (in_array($ext, $allowedExt, true)) {
            $files[] = $entry;
        }
    }
    sort($files);
}
 
header('Content-Type: text/html; charset=utf-8');
?>
<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="utf-8">
<title>Videos</title>
<link rel="stylesheet" href="/styles/common.css">
<link rel="stylesheet" href="/styles/watch.css">
</head>
<body class="page">
 
<div class="gallery">
    <h1 class="gallery__title">Videos</h1>
 
    <?php if (!$files): ?>
        <p class="gallery__empty">No videos found!</p>
    <?php else: ?>
        <div class="gallery__grid">
            <?php foreach ($files as $file):
                $name = pathinfo($file, PATHINFO_FILENAME);
            ?>
            <a class="video-card" href="watch.php?v=<?= urlencode($file) ?>">
                <video class="video-card__thumb" muted preload="metadata">
                    <source src="videos/<?= h(rawurlencode($file)) ?>">
                </video>
                <div class="video-card__name"><?= h($name) ?></div>
            </a>
            <?php endforeach; ?>
        </div>
    <?php endif; ?>
</div>
 
</body>
</html>
