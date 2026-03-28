--TEST--
Test tigger recursion causing large error message
--SKIPIF--
<?php include("skipif.inc"); ?>
--FILE--
<?php

require("interbase.inc");
ibase_connect($test_base);

(function(){
    $data = [];
    for ($i = 0; $i < 100; $i++) {
        $data["key$i"] = $i;
    }

    ibase_query(
        "CREATE TABLE T1 (
            ID INTEGER NOT NULL,
            CODE VARCHAR(100)
        )"
    );
    ibase_query(
        "CREATE TABLE T2 (
            ID INTEGER NOT NULL,
            CODE VARCHAR(100)
        )"
    );
    ibase_commit_ret();

    for ($i = 0; $i < 100; $i++) {
        ibase_query("INSERT INTO T1 (ID, CODE) VALUES ($i, '$i-$i-$i-$i')");
        ibase_query("INSERT INTO T2 (ID, CODE) VALUES ($i, '$i-$i-$i-$i')");
    }

    ibase_query(
        "CREATE OR ALTER TRIGGER T1_TRIGGER FOR T1
        BEFORE INSERT
        AS BEGIN
            INSERT INTO T2 (ID) VALUES (NULL);
        END"
    );
    ibase_query(
        "CREATE OR ALTER TRIGGER T2_TRIGGER FOR T2
        BEFORE INSERT
        AS BEGIN
            INSERT INTO T1 (ID) VALUES (NULL);
        END"
    );
    ibase_commit_ret();

    ibase_query("INSERT INTO T1 (ID) VALUES (NULL)");
})();

?>
--EXPECTF--
Warning: ibase_query(): Too many concurrent executions of the same request At trigger '%s' line: %d, col: %d
At trigger 'T1_TRIGGER' line: %d, col: %d
At trigger 'T2_TRIGGER' line: %d, col: %d
%a