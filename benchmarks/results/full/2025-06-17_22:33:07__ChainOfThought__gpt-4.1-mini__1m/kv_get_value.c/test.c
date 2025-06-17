#include "kv_file_get_value.h"
#include "kv_get_value.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

// Test case function
void run_kv_get_value_tests()
{
    char buffer[100] = {0};
    int buffer_count = 0;

    // **Test 1: Basic Key-Value Retrieval**
    memset(buffer, 0, sizeof(buffer));
    buffer_count = kv_get_value("key1=value1\nkey2=value2", "key1", buffer, sizeof(buffer));
    assert(buffer_count == 6);
    assert(strcmp(buffer, "value1") == 0);

    // **Test 2: Retrieve Last Key**
    memset(buffer, 0, sizeof(buffer));
    buffer_count = kv_get_value("a=b\nc=d\ne=f\ng=hello", "g", buffer, sizeof(buffer));
    assert(buffer_count == 5);
    assert(strcmp(buffer, "hello") == 0);

    // **Test 3: Key Not Found**
    memset(buffer, 0, sizeof(buffer));
    buffer_count = kv_get_value("a=b\nc=d", "z", buffer, sizeof(buffer));
    assert(buffer_count == 0);

    // **Test 4: Buffer Too Small**
    memset(buffer, 0, sizeof(buffer));
    buffer_count = kv_get_value("longkey=longvalue", "longkey", buffer, sizeof(buffer));
    assert(buffer_count == 9);
    assert(strcmp(buffer, "longvalue") == 0);

    // **Test 5: Empty Input**
    memset(buffer, 0, sizeof(buffer));
    buffer_count = kv_get_value("", "anykey", buffer, sizeof(buffer));
    assert(buffer_count == 0);

    // **Test 6: Input Without Key-Value Pairs**
    memset(buffer, 0, sizeof(buffer));
    buffer_count = kv_get_value("randomtext\nanotherline", "key", buffer, sizeof(buffer));
    assert(buffer_count == 0);

    //  **Test 7: Handling Spaces Around Key and Value**
    memset(buffer, 0, sizeof(buffer));
    buffer_count = kv_get_value(" key = value \n next = test ", "key", buffer, sizeof(buffer));
#ifndef KV_PARSE_DISABLE_WHITESPACE_SKIP
    assert(buffer_count == 5);
    assert(strcmp(buffer, "value") == 0);
#else
    assert(buffer_count == 0);
#endif

    // **Test 8: Duplicate Keys (Return First Occurrence)**
    memset(buffer, 0, sizeof(buffer));
    buffer_count = kv_get_value("x=1\nx=2\nx=3", "x", buffer, sizeof(buffer));
    assert(buffer_count == 1);
    assert(strcmp(buffer, "1") == 0);

    // **Test 9: Newline Variations (Windows vs. Unix)**
    memset(buffer, 0, sizeof(buffer));
    buffer_count = kv_get_value("a=one\r\nb=two", "b", buffer, sizeof(buffer));
    assert(buffer_count == 3);
    assert(strcmp(buffer, "two") == 0);

    // **Test 10: Key With Special Characters**
    memset(buffer, 0, sizeof(buffer));
    buffer_count = kv_get_value("user-name=admin\nuser@domain.com=me", "user-name", buffer, sizeof(buffer));
    assert(buffer_count == 5);
    assert(strcmp(buffer, "admin") == 0);

    // **Test 11: Value Containing '='**
    memset(buffer, 0, sizeof(buffer));
    buffer_count = kv_get_value("path=/home/user=data", "path", buffer, sizeof(buffer));
    assert(buffer_count == 15);
    assert(strcmp(buffer, "/home/user=data") == 0);

    // **Test 12: Quoted String **
    memset(buffer, 0, sizeof(buffer));
    buffer_count = kv_get_value("path=\"/home/user=data\"", "path", buffer, sizeof(buffer));
#ifndef KV_PARSE_DISABLE_QUOTED_STRINGS
    assert(buffer_count == 15);
    assert(strcmp(buffer, "/home/user=data") == 0);
#else
    assert(buffer_count == 17);
    assert(strcmp(buffer, "\"/home/user=data\"") == 0);
#endif

    // **Test 13: Uncapped Quoted String **
    memset(buffer, 0, sizeof(buffer));
    buffer_count = kv_get_value("path=\"/home/user=data", "path", buffer, sizeof(buffer));
#ifndef KV_PARSE_DISABLE_QUOTED_STRINGS
    assert(buffer_count == 15);
    assert(strcmp(buffer, "/home/user=data") == 0);
#else
    assert(buffer_count == 16);
    assert(strcmp(buffer, "\"/home/user=data") == 0);
#endif

    // **Test 14: Quoted String With Escaped Quote **
    memset(buffer, 0, sizeof(buffer));
    buffer_count = kv_get_value("path=\"/home/\\\"user=data\"", "path", buffer, sizeof(buffer));
#ifndef KV_PARSE_DISABLE_QUOTED_STRINGS
    assert(buffer_count == 16);
    assert(strcmp(buffer, "/home/\"user=data") == 0);
#else
    assert(buffer_count == 19);
    assert(strcmp(buffer, "\"/home/\\\"user=data\"") == 0);
#endif

    printf("kv_get_value() passed successfully!\n");
}

void run_kv_file_get_value_tests()
{
    // **Test 1: Basic Key-Value Retrieval**
    {
        char buffer[100] = {0};
        int buffer_count = 0;

        FILE *temp = tmpfile();
        assert(temp != NULL);

        fputs("key1=value1\nkey2=value2", temp);

        buffer_count = kv_file_get_value(temp, "key1", buffer, sizeof(buffer));
        fclose(temp);

        assert(buffer_count == 6);
        assert(strcmp(buffer, "value1") == 0);
    }

    // **Test 2: Retrieve Last Key**
    {
        char buffer[100] = {0};
        int buffer_count = 0;

        FILE *temp = tmpfile();
        assert(temp != NULL);

        fputs("a=b\nc=d\ne=f\ng=hello", temp);

        buffer_count = kv_file_get_value(temp, "g", buffer, sizeof(buffer));

        fclose(temp);

        assert(buffer_count == 5);
        assert(strcmp(buffer, "hello") == 0);
    }

    // **Test 3: Key Not Found**
    {
        char buffer[100] = {0};
        int buffer_count = 0;

        FILE *temp = tmpfile();
        assert(temp != NULL);

        fputs("a=b\nc=d", temp);

        buffer_count = kv_file_get_value(temp, "z", buffer, sizeof(buffer));

        fclose(temp);

        assert(buffer_count == 0);
    }

    // **Test 4: Buffer Too Small**
    {
        char buffer[100] = {0};
        int buffer_count = 0;

        FILE *temp = tmpfile();
        assert(temp != NULL);

        fputs("longkey=longvalue", temp);

        buffer_count = kv_file_get_value(temp, "longkey", buffer, sizeof(buffer));

        fclose(temp);

        assert(buffer_count == 9);
        assert(strcmp(buffer, "longvalue") == 0);
    }

    // **Test 5: Empty Input**
    {
        char buffer[100] = {0};
        int buffer_count = 0;

        FILE *temp = tmpfile();
        assert(temp != NULL);

        fputs("", temp);

        buffer_count = kv_file_get_value(temp, "anykey", buffer, sizeof(buffer));

        fclose(temp);

        assert(buffer_count == 0);
    }

    // **Test 6: Input Without Key-Value Pairs**
    {
        char buffer[100] = {0};
        int buffer_count = 0;

        FILE *temp = tmpfile();
        assert(temp != NULL);

        fputs("randomtext\nanotherline", temp);

        buffer_count = kv_file_get_value(temp, "key", buffer, sizeof(buffer));

        fclose(temp);

        assert(buffer_count == 0);
    }

    //  **Test 7: Handling Spaces Around Key and Value**
    {
        char buffer[100] = {0};
        int buffer_count = 0;

        FILE *temp = tmpfile();
        assert(temp != NULL);

        fputs(" key = value \n next = test ", temp);

        buffer_count = kv_file_get_value(temp, "key", buffer, sizeof(buffer));

        fclose(temp);

#ifndef KV_PARSE_DISABLE_WHITESPACE_SKIP
        assert(buffer_count == 5);
        assert(strcmp(buffer, "value") == 0);
#else
        assert(buffer_count == 0);
#endif
    }

    // **Test 8: Duplicate Keys (Return First Occurrence)**
    {
        char buffer[100] = {0};
        int buffer_count = 0;

        FILE *temp = tmpfile();
        assert(temp != NULL);

        fputs("x=1\nx=2\nx=3", temp);

        buffer_count = kv_file_get_value(temp, "x", buffer, sizeof(buffer));

        fclose(temp);

        assert(buffer_count == 1);
        assert(strcmp(buffer, "1") == 0);
    }

    // **Test 9: Newline Variations (Windows vs. Unix)**
    {
        char buffer[100] = {0};
        int buffer_count = 0;

        FILE *temp = tmpfile();
        assert(temp != NULL);

        fputs("a=one\r\nb=two", temp);

        buffer_count = kv_file_get_value(temp, "b", buffer, sizeof(buffer));

        fclose(temp);

        assert(buffer_count == 3);
        assert(strcmp(buffer, "two") == 0);
    }

    // **Test 10: Key With Special Characters**
    {
        char buffer[100] = {0};
        int buffer_count = 0;

        FILE *temp = tmpfile();
        assert(temp != NULL);

        fputs("user-name=admin\nuser@domain.com=me", temp);

        buffer_count = kv_file_get_value(temp, "user-name", buffer, sizeof(buffer));

        fclose(temp);

        assert(buffer_count == 5);
        assert(strcmp(buffer, "admin") == 0);
    }

    // **Test 11: Value Containing '='**
    {
        char buffer[100] = {0};
        int buffer_count = 0;

        FILE *temp = tmpfile();
        assert(temp != NULL);

        fputs("path=/home/user=data", temp);

        buffer_count = kv_file_get_value(temp, "path", buffer, sizeof(buffer));

        fclose(temp);

        assert(buffer_count == 15);
        assert(strcmp(buffer, "/home/user=data") == 0);
    }

    // **Test 12: Quoted String **
    {
        char buffer[100] = {0};
        int buffer_count = 0;

        FILE *temp = tmpfile();
        assert(temp != NULL);

        fputs("path=\"/home/user=data\"", temp);

        buffer_count = kv_file_get_value(temp, "path", buffer, sizeof(buffer));

        fclose(temp);

#ifndef KV_PARSE_DISABLE_QUOTED_STRINGS
        assert(buffer_count == 15);
        assert(strcmp(buffer, "/home/user=data") == 0);
#else
        assert(buffer_count == 17);
        assert(strcmp(buffer, "\"/home/user=data\"") == 0);
#endif
    }

    // **Test 13: Uncapped Quoted String **
    {
        char buffer[100] = {0};
        int buffer_count = 0;

        FILE *temp = tmpfile();
        assert(temp != NULL);

        fputs("path=\"/home/user=data", temp);

        buffer_count = kv_file_get_value(temp, "path", buffer, sizeof(buffer));

        fclose(temp);

#ifndef KV_PARSE_DISABLE_QUOTED_STRINGS
        assert(buffer_count == 15);
        assert(strcmp(buffer, "/home/user=data") == 0);
#else
        assert(buffer_count == 16);
        assert(strcmp(buffer, "\"/home/user=data") == 0);
#endif
    }

    // **Test 14: Quoted String With Escaped Quote **
    {
        char buffer[100] = {0};
        int buffer_count = 0;

        FILE *temp = tmpfile();
        assert(temp != NULL);

        fputs("path=\"/home/\\\"user=data\"", temp);

        buffer_count = kv_file_get_value(temp, "path", buffer, sizeof(buffer));

        fclose(temp);

#ifndef KV_PARSE_DISABLE_QUOTED_STRINGS
        assert(buffer_count == 16);
        assert(strcmp(buffer, "/home/\"user=data") == 0);
#else
        assert(buffer_count == 19);
        assert(strcmp(buffer, "\"/home/\\\"user=data\"") == 0);
#endif
    }

    printf("kv_file_get_value() passed successfully!\n");
}

// Run tests in main()
int main()
{
    run_kv_get_value_tests();
    run_kv_file_get_value_tests();
    printf("All tests passed successfully!\n");
    return 0;
}
