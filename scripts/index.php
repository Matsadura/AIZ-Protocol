<?php

$headers = getallheaders();

echo "<h1>Test cgi envirment variables</h1>";

echo "<h2>Quary values</h2>";
print_r($_GET);

echo "<h2>Headers</h2>";
echo "<dl>";
foreach ($headers as $key => $value)
{

	echo "<dt>" . $key . "</dt>";
	echo "<dd>" . $value . "</dd>";
}
echo "</dl>";

?>

