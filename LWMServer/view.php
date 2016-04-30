<?php
	function myErrHandler($error_level, $error_message){}
	
    $addr = "\\";
    
    function process_esc_char($str)
    {
        $ret = stripcslashes($str);
        $ret = str_replace(" ", "&nbsp;", $ret);
        $ret = str_replace("\t", "&nbsp;&nbsp;&nbsp;&nbsp;", $ret);
        $ret = str_replace("\n", "<br />", $ret);
        return $ret;
    }
    
	function query_list($field, $con)
	{
		$result = mysqli_query($con, "SELECT * FROM `" . $field . "`");
		if (!$result) die(mysqli_error());
		
		switch ($field)
		{
			case "group":
			case "member":
			case "work":
				while($row = mysqli_fetch_array($result))
				{
					echo "<a href=\"" . $addr . "?field=" . $field . "&id=" . $row['id'] . "\">" . $row['name'] . "</a><br />";
				}
				break;
			default:
				die("E:Invalid field");
		}
	}
	
	function split_list($field, $str, $con)
	{
		$units = explode(";", $str);
		if (!$units)
			return;
		
		foreach ($units as $id)
		{
			if ($id == "")
				break;
			$result = mysqli_query($con, "SELECT * FROM `" . $field . "` WHERE `id`=" . $id);
			if (!$result) die(mysqli_error());
			while($row = mysqli_fetch_array($result))
			{
				echo "<a href=\"" . $addr . "?field=" . $field . "&id=" . $id . "\">" . $row['name'] . "</a><br />";
			}
		}
	}
	
	function query_info($field, $id, $con)
	{
		$result = mysqli_query($con, "SELECT * FROM `" . $field . "` WHERE `id`=" . $id);
		if (!$result) die(mysqli_error());
		
		switch ($field)
		{
			case "group":
				while($row = mysqli_fetch_array($result))
				{
					echo "<p><b>Name:</b>" . $row['name'] . "</p>";
					echo "<p><b>Members:</b><br />";
					split_list('member', $row['member'], $con);
					echo "</p>";
				}
				break;
			case "member":
				while($row = mysqli_fetch_array($result))
				{
					echo "<p><b>Name:</b>&nbsp;" . $row['name'] . "<br /><b>Source:</b>&nbsp;" . $row['src'] . "<br /><b>Info:</b><br />" . process_esc_char($row['info']) . "</p>";
					echo "<p><b>Groups:</b><br />";
					split_list('group', $row['group'], $con);
					echo "</p>";
					echo "<p><b>Works:</b><br />";
					split_list('work', $row['work'], $con);
					echo "</p>";
				}
				break;
			case "work":
				while($row = mysqli_fetch_array($result))
				{
					echo "<p><b>Name:</b>" . $row['name'] . "<br /><b>Info:</b>" . process_esc_char($row['info']) . "</p>";
					echo "<p><b>Members:</b><br />";
					split_list('member', $row['member'], $con);
					echo "</p>";
				}
				break;
		}
	}
	
	set_error_handler('myErrHandler', E_NOTICE);
	$con = mysqli_connect("localhost:3306", "wmview", "", "workmanager");
	if (!$con) die(mysqli_error());
	if (isset($_GET["field"]))
	{
		echo "<p><a href=\"" . $addr . "\">Home</a></p>";
		//Run query
		query_info($_GET["field"], $_GET["id"], $con);
	}
	else
	{
		//Display list
		echo "<p><b>Groups:</b><br />";
		query_list("group", $con);
		echo "</p>";
		echo "<p><b>Members:</b><br />";
		query_list("member", $con);
		echo "</p>";
		echo "<p><b>Works:</b><br />";
		query_list("work", $con);
		echo "</p>";
	}
?>
