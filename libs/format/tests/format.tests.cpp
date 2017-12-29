#include <fstream>
#include <gtest/gtest.h>
#include <mira/format/format.hpp>

template <class... Args>
std::string usePosArgs(size_t idx, const Args&... args)
{
    using namespace mira::format::details;
    using Trait = TraitsImpl<char>;
    std::ostringstream      stream;
    Trait::PosArgs<Args...> pargs(args...);
    pargs.doOut(stream, idx);
    return stream.str();
}

std::string useNamedArgs(std::string_view key, const mira::format::details::TraitsImpl<char>::NamedArgs& args)
{
    std::ostringstream stream;
    args.doOut(stream, key);
    return stream.str();
}

TEST(Mira_Format_Details_Tests, PosArgs)
{
    std::string      mila = "mila";
    std::string_view ramu = "ramu";
    EXPECT_EQ("mama", usePosArgs(0, "mama", 1, mila, ramu, 2));
    EXPECT_EQ("1", usePosArgs(1, "mama", 1, mila, ramu, 2));
    EXPECT_EQ("mila", usePosArgs(2, "mama", 1, mila, ramu, 2));
    EXPECT_EQ("ramu", usePosArgs(3, "mama", 1, mila, ramu, 2));
    EXPECT_EQ("2", usePosArgs(4, "mama", 1, mila, ramu, 2));

    EXPECT_THROW(usePosArgs(5, "mama", 1, mila, ramu, 2), std::out_of_range);
}

TEST(Mira_Format_Details_Tests, NamedArgs)
{
    std::string      mila = "mila";
    std::string_view ramu = "ramu";
    EXPECT_EQ("mama", useNamedArgs("1", {{"1", "mama"}, {"2", 1}, {"3", mila}, {"4", ramu}, {"5", 2}}));
    EXPECT_EQ("1", useNamedArgs("2", {{"1", "mama"}, {"2", 1}, {"3", mila}, {"4", ramu}, {"5", 2}}));
    EXPECT_EQ("mila", useNamedArgs("3", {{"1", "mama"}, {"2", 1}, {"3", mila}, {"4", ramu}, {"5", 2}}));
    EXPECT_EQ("ramu", useNamedArgs("4", {{"1", "mama"}, {"2", 1}, {"3", mila}, {"4", ramu}, {"5", 2}}));
    EXPECT_EQ("2", useNamedArgs("5", {{"1", "mama"}, {"2", 1}, {"3", mila}, {"4", ramu}, {"5", 2}}));
}

TEST(Mira_Format_Tests, WithoutArgs)
{
    ASSERT_EQ("string", mira::fmt("string"));
}

TEST(Mira_Format_Tests, WithoutArgs_InputIsString)
{
    std::string input = "string";
    ASSERT_EQ("string", mira::fmt(input));
}

TEST(Mira_Format_Tests, NamedArgsOnly)
{
    ASSERT_EQ("mama mila ramu",
              mira::fmt("%{who} %{what_do} %{subject}", {{"who", "mama"}, {"what_do", "mila"}, {"subject", "ramu"}}));
}

TEST(Mira_Format_Tests, NamedArgsOnly_InputIsString)
{
    std::string input = "%{who} %{what_do} %{subject} %{digit}";
    std::string who   = "mama";
    std::string mila  = "mila";
    ASSERT_EQ("mama mila ramu 50",
              mira::fmt(input, {{"who", who}, {"what_do", mila}, {"subject", "ramu"}, {"digit", 50}}));
}

TEST(Mira_Format_Tests, PositionalArgsOnly)
{
    ASSERT_EQ("mama mila ramu", mira::fmt("%0 %1 %2", "mama", "mila", "ramu", 10, 20));
}

TEST(Mira_Format_Tests, PositionalArgsOnly_InputIsString)
{
    std::string input = "%0 %1 %2 %3 %4";
    std::string who   = "mama";
    std::string mila  = "mila";
    ASSERT_EQ("mama mila ramu 50 60", mira::fmt(input, who, mila, "ramu", 50, 60));
}

TEST(Mira_Format_Tests, MixedArgs)
{
    ASSERT_EQ("mama mila ramu", mira::fmt("%0 %{mila} %1", {{"mila", "mila"}}, "mama", "ramu", 10, 20));
}

TEST(Mira_Format_Tests, MixedArgs_InputIsString)
{
    std::string input = "%0 %{mila} %1 %{50} %2";
    std::string who   = "mama";
    std::string mila  = "mila";
    ASSERT_EQ("mama mila ramu 50 60", mira::fmt(input, {{"mila", "mila"}, {"50", 50}}, who, "ramu", 60));
}

TEST(Mira_Format_Tests, Double_Percent_Symbol)
{
    ASSERT_EQ("mama % mila % ramu", mira::fmt("%0 %% %1 %% %{ramu}", {{"ramu", "ramu"}}, "mama", "mila"));
}

TEST(Mira_Format_Tests, Unexpected_End_Percent_At_End)
{
    ASSERT_THROW(mira::fmt("%0 %% %1 %% %{ramu} %", {{"ramu", "ramu"}}, "mama", "mila"), std::invalid_argument);
}

TEST(Mira_Format_Tests, Unexpected_End_Unclosed_Curl_Braces)
{
    ASSERT_THROW(mira::fmt("%0 %% %1 %% %{ramu sfds", {{"ramu", "ramu"}}, "mama", "mila"), std::invalid_argument);
}

TEST(Mira_Format_Tests, Unexpected_Symbol_After_Percent)
{
    ASSERT_THROW(mira::fmt("%0 %% %1 %% %ramu sfds", {{"ramu", "ramu"}}, "mama", "mila"), std::invalid_argument);
}

TEST(Mira_Format_Tests, Unexpected_Empty_Key_For_NamedArg)
{
    ASSERT_THROW(mira::fmt("%0 %% %1 %% %{}", {{"ramu", "ramu"}}, "mama", "mila"), std::invalid_argument);
}
