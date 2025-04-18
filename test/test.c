
#include "test.h"

#include <stdio.h>

#include "../src/pattern.h"
#include "../src/util.h"
#include "../src/config.h"

static void test_patterns(void) {
    TEST_PATTERN(REGEX_URL, "http://example.com/");
    TEST_PATTERN(REGEX_URL, "https://example.com/");
    TEST_PATTERN(REGEX_PDF, "document.pdf");
    TEST_PATTERN(REGEX_PDF, "/tmp/document.pdf");
    TEST_PATTERN(REGEX_MAN, "mmap(2)");
    TEST_PATTERN(REGEX_LINECOL, "main.c:123");
    TEST_PATTERN(REGEX_LINECOL, "main.c:111:22");
    TEST_PATTERN(REGEX_LINECOL, "main.c:111:22: Error!");

    /*
     * TODO: Test patterns in 'editor_patterns', 'image_patterns' and
     * 'video_patterns'.
     */
}

int main(void) {
    test_patterns();
    puts("[test] Passed pattern tests.");

    puts("[test] Success: All tests passed.");
    return 0;
}
