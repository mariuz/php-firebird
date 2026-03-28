--TEST--
Test 1000+ rows to trigger "Too many concurrent executions of the same request" bug #101
--SKIPIF--
<?php include("skipif.inc"); ?>
--FILE--
<?php

require("interbase.inc");
ibase_connect($test_base);

(function(){
    ibase_query(
        "CREATE TABLE ITEMS (
            ID INTEGER NOT null,
            CODE1 VARCHAR(32) CHARACTER SET NONE,
            CODE10 VARCHAR(32) CHARACTER SET NONE,
            MAN_ID INTEGER
        )"
    );
    ibase_commit_ret();

    $data = [
        [1, "CODE1 1", "CODE10 1", null],
        [2, null, "CODE10 2", 101],
        [3, "CODE1 3", null, null],
        [4, null, null, 104],
        [5, "CODE1 5", null, 105],
        [6, "CODE1 6", "CODE10 6", null],
        [7, null, "CODE10 7", 107],
        [8, null, null, null],
        [9, "CODE1 8", "CODE10 9", 109],
    ];

    $sql =
        "INSERT INTO ITEMS (
            ID, CODE1, CODE10, MAN_ID
        ) VALUES (
        ?, ?, ?, ?
        )";

    $c = 0;
    $total_data = count($data);
    for ($i = 0; $i < 1001; $i++) {
        ibase_query($sql, ...$data[$i % $total_data]);
    }

    print "OK";
})();

?>
--EXPECT--
OK