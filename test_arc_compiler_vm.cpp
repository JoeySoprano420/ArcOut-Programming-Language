#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <stdexcept>
#include <cstdlib>

void writeSourceFile(const std::string& code, const std::string& filename = "test.arc") {
    std::ofstream file(filename);
    if (!file) {
        throw std::runtime_error("Failed to open file for writing.");
    }
    file << code;
}

std::string compileAndRun() {
    // Compile the source file
    if (std::system("python3 ArcCompilerVm.py arcbuild test.arc") != 0) {
        throw std::runtime_error("Compilation failed.");
    }

    // Run the compiled file and capture the output
    std::ostringstream command;
    command << "python3 ArcCompilerVm.py arcrun main.arx";
    FILE* pipe = popen(command.str().c_str(), "r");
    if (!pipe) {
        throw std::runtime_error("Failed to run the program.");
    }

    char buffer[128];
    std::ostringstream output;
    while (fgets(buffer, sizeof(buffer), pipe)) {
        output << buffer;
    }
    pclose(pipe);

    return output.str();
}

void runTestCase(const std::string& name, const std::string& code, const std::string& expectedOutput) {
    std::cout << "\n=== Running " << name << " ===" << std::endl;

    try {
        writeSourceFile(code);
        std::string output = compileAndRun();

        // Check if the expected output exists in the actual output
        if (output.find(expectedOutput) == std::string::npos) {
            throw std::runtime_error("❌ " + name + " failed!\nExpected: " + expectedOutput + "\nGot: " + output);
        }

        std::cout << "✅ " << name << " passed." << std::endl;
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
    }
}

int main() {
    // --- TEST CASES ---
    runTestCase(
        "Test_Addition",
        R"(
        let a = 5
        let b = 7
        c = a + b
        return c
        )",
        "Return: 12"
    );

    runTestCase(
        "Test_Subtraction",
        R"(
        let a = 10
        let b = 3
        c = a - b
        return c
        )",
        "Return: 7"
    );

    runTestCase(
        "Test_While_Loop",
        R"(
        let a = 0
        let b = 5
        while a < b {
            a = a + 1
        }
        return a
        )",
        "Return: 5"
    );

    runTestCase(
        "Test_If_Condition_True",
        R"(
        let x = 9
        if x > 5 {
            x = x + 2
        }
        return x
        )",
        "Return: 11"
    );

    runTestCase(
        "Test_If_Condition_False",
        R"(
        let x = 2
        if x > 5 {
            x = x + 2
        }
        return x
        )",
        "Return: 2"
    );

    runTestCase(
        "Test_Equality",
        R"(
        let a = 4
        let b = 4
        let result = a == b
        return result
        )",
        "Return: true"
    );

    runTestCase(
        "Test_Inequality",
        R"(
        let x = 5
        let y = 3
        let diff = x != y
        return diff
        )",
        "Return: true"
    );

    runTestCase(
        "Test_Boolean_Negation",
        R"(
        let a = true
        let b = !a
        return b
        )",
        "Return: false"
    );

    runTestCase(
        "Test_Nested_If",
        R"(
        let a = 10
        if a > 5 {
            if a < 15 {
                a = a + 5
            }
        }
        return a
        )",
        "Return: 15"
    );

    runTestCase(
        "Test_While_With_Condition",
        R"(
        let i = 0
        while i < 10 {
            if i == 4 {
                break
            }
            i = i + 1
        }
        return i
        )",
        "Return: 4"
    );

    runTestCase(
        "Test_Chain_Assignment",
        R"(
        let a = 1
        let b = a
        let c = b
        return c
        )",
        "Return: 1"
    );

    runTestCase(
        "Test_Multiple_Returns",
        R"(
        let a = 3
        return a
        return 99
        )",
        "Return: 3"
    );

    runTestCase(
        "Test_Division",
        R"(
        let x = 20
        let y = 4
        let result = x / y
        return result
        )",
        "Return: 5"
    );

    runTestCase(
        "Test_Multiplication",
        R"(
        let x = 6
        let y = 7
        return x * y
        )",
        "Return: 42"
    );

    runTestCase(
        "Test_Complex_Expression",
        R"(
        let x = 10
        let y = 5
        let z = 2
        let r = (x + y) * z - 5
        return r
        )",
        "Return: 25"
    );

    return 0;
}
