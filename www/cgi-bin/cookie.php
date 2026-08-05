<?php
// PHP-CGI requires HTTP headers to be emitted before any body output.
header("Content-Type: text/plain; charset=UTF-8");

$cookieName = 'custom_session_id';
$sessionId = null;

// Check if a valid session cookie was sent in the HTTP request
if (isset($_COOKIE[$cookieName]) && preg_match('/^[a-f0-9]{64}$/', $_COOKIE[$cookieName])) {
    $sessionId = $_COOKIE[$cookieName];
    echo "Existing Session ID: " . $sessionId . "\n";
} else {
    // Generate a cryptographically secure 256-bit (64 hex character) session ID
    $sessionId = bin2hex(random_bytes(32));

    // Define cookie attributes for security and scope
    $cookieOptions = [
        'expires'  => time() + 10, // Expire in 1 hour
        'path'     => '/',
        'domain'   => '',            // Defaults to the host of the current request
        'secure'   => true,          // Transmit over HTTPS only
        'httponly' => false,          // Prevent client-side JavaScript access
        'samesite' => 'Strict'       // Prevent cross-site request forgery (CSRF)
    ];

    // Emit the Set-Cookie header
    setcookie($cookieName, $sessionId, $cookieOptions);

    echo "New Session ID generated: " . $sessionId . "\n";
}
?>