#include <gtest/gtest.h>

#include <lstk/lstk.hpp>


TEST(split_on, single_qubit_gate_split)
{
    std::string input = "h q[1];";
    std::vector<std::string_view> splits1 = lstk::split_on(input,'[');
    ASSERT_EQ("h q",splits1.at(0));
    ASSERT_EQ("1];",splits1.at(1));
    ASSERT_EQ("1",lstk::split_on(
            lstk::split_on(input,'[').at(1),
            ']'
    ).at(0));

}

TEST(split_on, no_trailing_separator)
{
    std::string input = "a,b,z";
    std::vector<std::string_view> splits1 = lstk::split_on(input,',');
    ASSERT_EQ(3,splits1.size());
    ASSERT_EQ("a",splits1.at(0));
    ASSERT_EQ("b",splits1.at(1));
    ASSERT_EQ("c",splits1.at(2));
}

TEST(split_on, with_trailing_separator)
{
    std::string input = "a,b,c,";
    std::vector<std::string_view> splits1 = lstk::split_on(input,',');
    ASSERT_EQ(4,splits1.size());
    ASSERT_EQ("a",splits1.at(0));
    ASSERT_EQ("b",splits1.at(1));
    ASSERT_EQ("c",splits1.at(2));
    ASSERT_EQ("",splits1.at(3));
}