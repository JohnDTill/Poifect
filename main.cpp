#include <iostream>
#include <fstream>

#include "hashbenchmark.h"
#include "hashsearch.h"
#include "hashsearch2.h"

static std::vector<std::string> cpp_keywords {
    "alignas", //(since C++11)
    "alignof", //(since C++11)
    "and",
    "and_eq",
    "asm",
    "atomic_cancel", //(TM TS)
    "atomic_commit", //(TM TS)
    "atomic_noexcept", //(TM TS)
    "auto",
    "bitand",
    "bitor",
    "bool",
    "break",
    "case",
    "catch",
    "char",
    "char8_t", //(since C++20)
    "char16_t", //(since C++11)
    "char32_t", //(since C++11)
    "class",
    "compl",
    "concept", //(since C++20)
    "const",
    "consteval", //(since C++20)
    "constexpr", //(since C++11)
    "constinit", //(since C++20)
    "const_cast",
    "continue",
    "co_await", //(since C++20)
    "co_return", //(since C++20)
    "co_yield", //(since C++20)
    "decltype", //(since C++11)
    "default",
    "delete",
    "do",
    "double",
    "dynamic_cast",
    "else",
    "enum",
    "explicit",
    "export",
    "extern",
    "false",
    "float",
    "for",
    "friend",
    "goto",
    "if",
    "inline",
    "int",
    "long",
    "mutable",
    "namespace",
    "new",
    "noexcept", //(since C++11)
    "not",
    "not_eq",
    "nullptr", //(since C++11)
    "operator",
    "or",
    "or_eq",
    "private",
    "protected",
    "public",
    "reflexpr", //(reflection TS)
    "register",
    "reinterpret_cast",
    "requires", //(since C++20)
    "return",
    "short",
    "signed",
    "sizeof",
    "static",
    "static_assert", //(since C++11)
    "static_cast",
    "struct",
    "switch",
    "synchronized", //(TM TS)
    "template",
    "this",
    "thread_local", //(since C++11)
    "throw",
    "true",
    "try",
    "typedef",
    "typeid",
    "typename",
    "union",
    "unsigned",
    "using",
    "virtual",
    "void",
    "volatile",
    "wchar_t",
    "while",
    "xor",
    "xor_eq",
};

static std::vector<std::string> makeKeywordVals(){
    std::vector<std::string> vals;
    for(std::string keyword : cpp_keywords){
        std::transform(keyword.begin(), keyword.end(), keyword.begin(), toupper);
        vals.push_back(keyword);
    }

    return vals;
}
static std::vector<std::string> cpp_vals = makeKeywordVals();

static std::vector<std::string> greek_keywords {
    "alpha",
    "Alpha",
    "beta",
    "Beta",
    "chi",
    "Chi",
    "delta",
    "Delta",
    "epsilon",
    "Epsilon",
    "eta",
    "Eta",
    "gamma",
    "Gamma",
    "iota",
    "Iota",
    "kappa",
    "Kappa",
    "lambda",
    "Lambda",
    "mu",
    "Mu",
    "nu",
    "Nu",
    "omega",
    "Omega",
    "omicron",
    "Omicron",
    "phi",
    "Phi",
    "pi",
    "Pi",
    "psi",
    "Psi",
    "rho",
    "Rho",
    "sigma",
    "Sigma",
    "tau",
    "Tau",
    "theta",
    "Theta",
    "upsilon",
    "Upsilon",
    "xi",
    "Xi",
    "zeta",
    "Zeta",
};

static std::vector<std::string> greek_vals {
    "α",
    "Α",
    "β",
    "Β",
    "χ",
    "Χ",
    "δ",
    "Δ",
    "ϵ",
    "Ε",
    "η",
    "Η",
    "γ",
    "Γ",
    "ι",
    "Ι",
    "κ",
    "Κ",
    "λ",
    "Λ",
    "μ",
    "Μ",
    "ν",
    "Ν",
    "ω",
    "Ω",
    "ο",
    "Ο",
    "ϕ",
    "Φ",
    "π",
    "Π",
    "ψ",
    "Ψ",
    "ρ",
    "Ρ",
    "σ",
    "Σ",
    "τ",
    "Τ",
    "θ",
    "Θ",
    "υ",
    "Υ",
    "ξ",
    "Ξ",
    "ζ",
    "Ζ",
};

static constexpr uint16_t symbolsToInt(char a, char b){
    return a + (b << 8);
}

std::vector<uint16_t> symbols = {
    symbolsToInt('-', '>'),
    symbolsToInt('<', '-'),
    symbolsToInt('=', '>'),
    symbolsToInt('<', '='),
    symbolsToInt('=', '/'),
    symbolsToInt('=', '_'),
    symbolsToInt('>', '/'),
    symbolsToInt('<', '/'),
    symbolsToInt('/', '0'),
    symbolsToInt('/', '\\'),
    symbolsToInt('\\', '/'),
    symbolsToInt('|', '-'),
    symbolsToInt('|', '|'),
    symbolsToInt('~', '~'),
    symbolsToInt(':', '='),
    symbolsToInt('+', '_'),
    symbolsToInt(':', ':'),
    symbolsToInt('-', ':'),
    symbolsToInt('<', '<'),
    symbolsToInt('>', '>'),
};

std::vector<std::string> symbol_vals = {
    "→",
    "←",
    "⇒",
    "⇐",
    "≠",
    "≡",
    "≯",
    "≮",
    "∅",
    "∧",
    "∨",
    "⊢",
    "‖",
    "≈",
    "≔",
    "±",
    "∷",
    "∹",
    "≪",
    "≫",
};

void saveToFile(const std::string& str, const std::string& filename){
    std::ofstream out(SRC"/" +filename);
    assert(out.is_open());
    out << str;
}

//#define BOOTSTRAPPED
#ifdef BOOTSTRAPPED
#include "poifect_adhocsymbols.h"
#include "poifect_adhocsymbols2.h"
#include "poifect_cppkeywords.h"
#include "poifect_cppkeywords2.h"
#include "poifect_greekletters.h"
#include "poifect_greekletters2.h"

#define NDEBUG
#include "poifect_adhocsymbols_keyonly.h"
#include "poifect_adhocsymbols2_keyonly.h"

void checkPreviouslyGeneratedResults(){
    //This tests the previously generated results
    assert( CppKeywords::lookup("operator") == "OPERATOR" );
    assert( CppKeywords::lookup("operatee") == "IDENTIFIER" );
    assert( CppKeywords2::lookup("operator") == "OPERATOR" );
    assert( CppKeywords2::lookup("operatee") == "IDENTIFIER" );

    assert( AdhocSymbols::lookup(symbolsToInt('-', '>')) == "→" );
    assert( AdhocSymbols::lookup(symbolsToInt('@', '!')) == "" );
    assert( AdhocSymbols2::lookup(symbolsToInt('-', '>')) == "→" );
    assert( AdhocSymbols2::lookup(symbolsToInt('@', '!')) == "" );

    assert( GreekLetters::lookup("pi") == "π" );
    assert( GreekLetters::lookup("vhi") == "" );
    assert( GreekLetters2::lookup("pi") == "π" );
    assert( GreekLetters2::lookup("vhi") == "" );

    for(size_t i = 0; i < cpp_keywords.size(); i++){
        assert(CppKeywords::lookup(cpp_keywords[i]) == cpp_vals[i]);
        assert(CppKeywords2::lookup(cpp_keywords[i]) == cpp_vals[i]);
    }

    for(size_t i = 0; i < greek_keywords.size(); i++){
        assert(GreekLetters::lookup(greek_keywords[i]) == greek_vals[i]);
        assert(GreekLetters2::lookup(greek_keywords[i]) == greek_vals[i]);
    }

    for(size_t i = 0; i < symbols.size(); i++){
        assert(AdhocSymbols::lookup(symbols[i]) == symbol_vals[i]);
        assert(AdhocSymbols2::lookup(symbols[i]) == symbol_vals[i]);
    }

    std::cout << "TESTS SUCCESSFUL\n" << std::endl;

    std::cout << "CppKeyword keys: ";
    runBenchmark<CppKeywords>(cpp_keywords);
    std::cout << "CppKeyword non-keys: ";
    runBenchmark<CppKeywords>(greek_keywords);

    std::cout << "CppKeyword2 keys: ";
    runBenchmark<CppKeywords2>(cpp_keywords);
    std::cout << "CppKeyword2 non-keys: ";
    runBenchmark<CppKeywords2>(greek_keywords);

    std::cout << "GreekLetters keys: ";
    runBenchmark<GreekLetters>(greek_keywords);
    std::cout << "GreekLetters non-keys: ";
    runBenchmark<GreekLetters>(cpp_keywords);

    std::cout << "GreekLetters2 keys: ";
    runBenchmark<GreekLetters2>(greek_keywords);
    std::cout << "GreekLetters2 non-keys: ";
    runBenchmark<GreekLetters2>(cpp_keywords);

    std::cout << "AdhocSymbol keys: ";
    runBenchmark<AdhocSymbols>(symbols);
    std::cout << "AdhocSymbol2 keys: ";
    runBenchmark<AdhocSymbols2>(symbols);
    std::cout << "AdhocSymbol keys only: ";
    runBenchmark<AdhocSymbolsKeyOnly>(symbols);
    std::cout << "AdhocSymbol2 keys only: ";
    runBenchmark<AdhocSymbols2KeyOnly>(symbols);
}
#endif

int main(){
    std::string hash_str;
    bool success;

    success = hashSearch2<std::string>(cpp_keywords, cpp_vals, hash_str, "CppKeywords2", "IDENTIFIER", 1, 4);
    assert(success);
    saveToFile(hash_str, "poifect_cppkeywords2.h");
    success = hashSearch2<std::string>(greek_keywords, greek_vals, hash_str, "GreekLetters2");
    assert(success);
    saveToFile(hash_str, "poifect_greekletters2.h");
    success = hashSearch2<uint16_t>(symbols, symbol_vals, hash_str, "AdhocSymbols2", "", 1, 6);
    assert(success);
    saveToFile(hash_str, "poifect_adhocsymbols2.h");
    success = hashSearch2<uint16_t>(symbols, symbol_vals, hash_str, "AdhocSymbols2KeyOnly", "", 1, 6, false);
    assert(success);
    saveToFile(hash_str, "poifect_adhocsymbols2_keyonly.h");
    success = hashSearch<std::string>(cpp_keywords, cpp_vals, hash_str, "CppKeywords", "IDENTIFIER", 3);
    assert(success);
    saveToFile(hash_str, "poifect_cppkeywords.h");
    success = hashSearch<std::string>(greek_keywords, greek_vals, hash_str, "GreekLetters", "", 2);
    assert(success);
    saveToFile(hash_str, "poifect_greekletters.h");
    success = hashSearch<uint16_t>(symbols, symbol_vals, hash_str, "AdhocSymbols");
    assert(success);
    saveToFile(hash_str, "poifect_adhocsymbols.h");
    success = hashSearch<uint16_t>(symbols, symbol_vals, hash_str, "AdhocSymbolsKeyOnly", "", 1, 1, false);
    assert(success);
    saveToFile(hash_str, "poifect_adhocsymbols_keyonly.h");

    #ifdef BOOTSTRAPPED
    checkPreviouslyGeneratedResults();
    #endif

    std::cout << "Analyzing greek keywords: ";
    analyze(greek_keywords);

    return 0;
}
