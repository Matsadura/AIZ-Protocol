<?php
// PHP-CGI requires HTTP headers to be emitted before any body output.
header("Content-Type: text/html; charset=UTF-8");

$cookieName = 'custom_session_id';
$userDbFile = __DIR__ . '/users.txt';
$sessionDbFile = __DIR__ . '/sessions.txt';
$message = '';
$isAuthenticated = false;

// Create the users database file if it does not exist
// Format: username:password
if (!file_exists($userDbFile)) {
    file_put_contents($userDbFile, "admin:password\nuser1:secret123\n");
}

// Ensure the sessions file exists
// Format: session_id:username:timestamp
if (!file_exists($sessionDbFile)) {
    touch($sessionDbFile);
}

if ($_SERVER['REQUEST_METHOD'] === 'POST') {
    $inputUsername = trim($_POST['username'] ?? '');
    $inputPassword = $_POST['password'] ?? '';

    $lines = file($userDbFile, FILE_IGNORE_NEW_LINES | FILE_SKIP_EMPTY_LINES);
    
    foreach ($lines as $line) {
        $parts = explode(':', $line, 2);
        
        if (count($parts) === 2) {
            $dbUsername = trim($parts[0]);
            $dbPassword = trim($parts[1]);

            if ($inputUsername === $dbUsername && $inputPassword === $dbPassword) {
                $isAuthenticated = true;
                break;
            }
        }
    }

    if ($isAuthenticated) {
        // Generate a 256-bit hexadecimal session ID
        $sessionId = bin2hex(random_bytes(32));
        $timestamp = time();

        // Store the session ID, username, and creation timestamp in the sessions file
        // Using FILE_APPEND and LOCK_EX to prevent concurrent write corruption
        $sessionRecord = $sessionId . ':' . $inputUsername . ':' . $timestamp . "\n";
        file_put_contents($sessionDbFile, $sessionRecord, FILE_APPEND | LOCK_EX);

        $cookieOptions = [
            'expires'  => $timestamp + 3600,
            'path'     => '/',
            'domain'   => '',
            'secure'   => true,
            'httponly' => false,
            'samesite' => 'Strict'
        ];

        setcookie($cookieName, $sessionId, $cookieOptions);
        $message = "Authentication successful. Session ID generated, stored, and cookie sent.";
    } else {
        $message = "Invalid username or password.";
    }
}
?>
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <title>Login Authentication</title>
</head>
<body>
    <h1>Login</h1>
    
    <?php if (!empty($message)): ?>
        <p><?php echo htmlspecialchars($message, ENT_QUOTES, 'UTF-8'); ?></p>
    <?php endif; ?>

    <?php if (!$isAuthenticated): ?>
        <form method="POST" action="">
            <div>
                <label>Username: <input type="text" name="username" required></label>
            </div>
            <div>
                <label>Password: <input type="password" name="password" required></label>
            </div>
            <div>
                <button type="submit">Log In</button>
            </div>
        </form>
        <p>Test credentials &mdash; Username: <code>admin</code> | Password: <code>password</code></p>
    <?php else: ?>
        <p>You are authenticated. You can now visit your HTML/JS page to inspect the session cookie.</p>
    <?php endif; ?>
</body>
</html>