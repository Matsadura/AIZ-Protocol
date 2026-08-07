<?php

session_start(["save_path" => "../sessions"]);

if (!isset($_SESSION['todos'])) {
    $_SESSION['todos'] = ["Laundry", "Session management"];
}

if ($_SERVER['REQUEST_METHOD'] === 'POST' && isset($_POST['new_todo'])) {
    $new_todo = trim($_POST['new_todo']);
    if (!empty($new_todo)) {
        $_SESSION['todos'][] = htmlspecialchars($new_todo);
    }
    header('Location: ' . $_SERVER['PHP_SELF'], true, 303);
    exit;
}

if ($_SERVER['REQUEST_METHOD'] === 'POST' && isset($_POST['delete_index'])) {
    $index = (int)$_POST['delete_index'];
    if (isset($_SESSION['todos'][$index])) {
        array_splice($_SESSION['todos'], $index, 1);
    }
    header('Location: ' . $_SERVER['PHP_SELF'], true, 303);
    exit;
}

$todos = $_SESSION['todos'];

?>
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <title>Todo List</title>
    <!-- Open Props CDN -->
    <link rel="stylesheet" href="/styles/common.css">
    <link rel="stylesheet" href="/styles/session.css">
</head>
<body class="page">
    <main class="todo-widget">
        <h1 class="todo-widget__title">Todo List</h1>
        
        <ul class="todo-widget__list">
            <?php if (empty($todos)): ?>
                <li class="todo-widget__empty">No todos yet. Add one below.</li>
            <?php else: ?>
                <?php foreach ($todos as $index => $todo): ?>
                    <li class="todo-item">
                        <span class="todo-item__text"><?php echo $todo; ?></span>
                        
                        <form method="POST" class="todo-item__form">
                            <input type="hidden" name="delete_index" value="<?php echo $index; ?>">
                            <button type="submit" class="btn btn--delete">Delete</button>
                        </form>
                    </li>
                <?php endforeach; ?>
            <?php endif; ?>
        </ul>

        <form method="POST" class="todo-form">
            <textarea 
                name="new_todo" 
                class="todo-form__input" 
                placeholder="Enter new todo..." 
                rows="2"
                required
            ></textarea>
            <button type="submit" class="btn btn--add">Add</button>
        </form>

        <hr class="todo-widget__divider">
        <p class="todo-widget__meta">Session ID: <?php echo session_id(); ?></p>
        <p class="todo-widget__meta">Total todos: <?php echo count($todos); ?></p>
    </main>
</body>
</html>
