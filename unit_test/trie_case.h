/*
 * Copyright (c) 2014-2015, dennis wang
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL dennis wang BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "helper.h"
#include "knet.h"

CASE(Test_Trie_Insert) {
    ktrie_t* t = trie_create();
    EXPECT_TRUE(error_ok == trie_insert(t, "abc", 0));
    EXPECT_FALSE(error_ok == trie_insert(t, "abc", 0));
    EXPECT_TRUE(error_ok == trie_insert(t, "abcd", 0));
    EXPECT_TRUE(error_ok == trie_insert(t, "ef", 0));
    EXPECT_TRUE(error_ok == trie_find(t, "ef", 0));
    trie_destroy(t, 0);
}

CASE(Test_Trie_Find) {
    int i = 10;
    void* iptr = 0;
    ktrie_t* t = trie_create();
    EXPECT_TRUE(error_ok == trie_insert(t, "abc", &i));
    EXPECT_TRUE(error_ok == trie_find(t, "abc", &iptr));
    EXPECT_TRUE(&i == iptr);
    EXPECT_FALSE(error_ok == trie_find(t, "abd", 0));
    EXPECT_FALSE(error_ok == trie_find(t, "abcd", 0));
    EXPECT_FALSE(error_ok == trie_find(t, "ab", &iptr));
    EXPECT_FALSE(iptr);
    EXPECT_FALSE(error_ok == trie_find(t, "a", &iptr));
    EXPECT_FALSE(iptr);
    trie_destroy(t, 0);
}

CASE(Test_Trie_Remove) {
    ktrie_t* t = trie_create();
    EXPECT_TRUE(error_ok == trie_insert(t, "abc", 0));
    EXPECT_FALSE(error_ok == trie_remove(t, "a", 0));
    EXPECT_FALSE(error_ok == trie_remove(t, "ab", 0));
    EXPECT_TRUE(error_ok == trie_remove(t, "abc", 0));
    EXPECT_FALSE(error_ok == trie_find(t, "abc", 0));
    EXPECT_FALSE(error_ok == trie_remove(t, "efg", 0));

    EXPECT_TRUE(error_ok == trie_insert(t, "abcdef", 0));
    EXPECT_TRUE(error_ok == trie_insert(t, "def", 0));
    EXPECT_TRUE(error_ok == trie_remove(t, "def", 0));
    trie_destroy(t, 0);
}
