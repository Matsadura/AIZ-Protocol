<?php

if ($_SERVER['REQUEST_METHOD'] === 'POST' && isset($_POST['action'])) {
    if ($_POST['action'] === 'set') {
        $key   = trim($_POST['cookie_key'] ?? '');
        $value = trim($_POST['cookie_value'] ?? '');

        if (!empty($key) && preg_match('/^[a-zA-Z0-9_-]+$/', $key)) {
            setcookie($key, $value, time() + 3600, "/", "", false, true);
        }
    } elseif ($_POST['action'] === 'delete' && isset($_POST['cookie_key'])) {
        $key = $_POST['cookie_key'];
        setcookie($key, "", time() - 3600, "/");
    }

    header('Location: ' . $_SERVER['PHP_SELF'], true, 303);
    exit;
}

?>
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <title>Cookie Manager</title>
    <!-- Open Props CDN -->
    <link rel="stylesheet" href="/styles/common.css">
    <link rel="stylesheet" href="/styles/cookie.css">
    <style>
    </style>
</head>
<body class="page">
    <main class="cookie-widget">
        <h1 class="cookie-widget__title">Cookie Manager</h1>
        <p class="cookie-widget__subtitle">Set custom HTTP cookies and inspect active headers.</p>

        <!-- Form to set a new cookie via POST -->
        <form method="POST" class="cookie-form">
            <input type="hidden" name="action" value="set">
            <div class="cookie-form__row">
                <input 
                    type="text" 
                    name="cookie_key" 
                    class="cookie-form__input" 
                    placeholder="Key (e.g., theme)" 
                    pattern="[a-zA-Z0-9_-]+"
                    title="Alphanumeric, dashes, and underscores only"
                    required
                >
                <input 
                    type="text" 
                    name="cookie_value" 
                    class="cookie-form__input" 
                    placeholder="Value (e.g., dark)" 
                    required
                >
            </div>
            <button type="submit" class="btn btn--add">Set Cookie</button>
        </form>

        <hr class="cookie-widget__divider">

        <h2 class="cookie-widget__title" style="font-size: var(--font-size-3);">Active Browser Cookies</h2>
        
        <!-- Display all incoming cookies from $_COOKIE -->
        <ul class="cookie-list">
            <?php if (empty($_COOKIE)): ?>
                <li class="cookie-widget__empty">No cookies sent by the browser.</li>
            <?php else: ?>
                <?php foreach ($_COOKIE as $key => $val): ?>
                    <li class="cookie-item">
                        <div>
                            <span class="cookie-item__key"><?php echo htmlspecialchars($key); ?>:</span>
                            <span class="cookie-item__val"><?php echo htmlspecialchars($val); ?></span>
                        </div>
                        
                        <!-- Form to delete an existing cookie -->
                        <form method="POST" style="margin: 0;">
                            <input type="hidden" name="action" value="delete">
                            <input type="hidden" name="cookie_key" value="<?php echo htmlspecialchars($key); ?>">
                            <button type="submit" class="btn btn--delete">Delete</button>
                        </form>
                    </li>
                <?php endforeach; ?>
            <?php endif; ?>
        </ul>
    </main>
</body>
</html>
