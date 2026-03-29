--TEST--
Test NULLs
--SKIPIF--
<?php include("skipif.inc"); ?>
--FILE--
<?php

require("interbase.inc");
ibase_connect($test_base);

(function(){
    ibase_query(
        "CREATE TABLE ITEMS (
            ID INTEGER,
            CODE VARCHAR(5) CHARACTER SET NONE,
            MAN_ID INTEGER
        )"
    );
    ibase_commit_ret();

    $data = [
        [1, "1", 10],
        [2, "2", 20],
        [3, "", NULL],
        [4, "4", 40],
        [5, "", NULL],
    ];

    $p = ibase_prepare("INSERT INTO ITEMS (ID, CODE, MAN_ID) VALUES (?, ?, ?)");

    foreach ($data as $row) {
        ibase_execute($p, ...$row);
    }

    dump_table_rows("ITEMS");
})();

?>
--EXPECT--
array(3) {
  ["ID"]=>
  int(1)
  ["CODE"]=>
  string(1) "1"
  ["MAN_ID"]=>
  int(10)
}
array(3) {
  ["ID"]=>
  int(2)
  ["CODE"]=>
  string(1) "2"
  ["MAN_ID"]=>
  int(20)
}
array(3) {
  ["ID"]=>
  int(3)
  ["CODE"]=>
  string(0) ""
  ["MAN_ID"]=>
  NULL
}
array(3) {
  ["ID"]=>
  int(4)
  ["CODE"]=>
  string(1) "4"
  ["MAN_ID"]=>
  int(40)
}
array(3) {
  ["ID"]=>
  int(5)
  ["CODE"]=>
  string(0) ""
  ["MAN_ID"]=>
  NULL
}