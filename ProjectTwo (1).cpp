/**
 * @file ProjectTwo.cpp
 * @author Erica Kinch
 * @date 2026-10-18
 * @brief Command-line course advising application for CS 300 Project Two.
 *
 * Reads comma-separated course records into a binary search tree ordered by
 * course number. Supports sorted listings, prerequisite lookup, and an optional
 * prerequisite-valid completion schedule using Kahn's topological sort.
 * Requires C++14 or later and uses only standard-library headers.
 * Input format: courseNumber,courseTitle,prerequisite1,prerequisite2,...
 * Empty prerequisite fields are ignored to support the supplied input file.
 */

#include <algorithm>
#include <cctype>
#include <fstream>
#include <iostream>
#include <memory>
#include <queue>
#include <sstream>
#include <string>
#include <utility>
#include <unordered_map>
#include <vector>

struct Course
{
    std::string number;
    std::string title;
    std::vector<std::string> prerequisites;
};

std::string trim(const std::string &text)
{
    const auto first = text.find_first_not_of(" \t\r\n");
    if (first == std::string::npos)
    {
        return "";
    }
    const auto last = text.find_last_not_of(" \t\r\n");
    return text.substr(first, last - first + 1);
}

std::string normalizeNumber(std::string number)
{
    number = trim(number);
    // Unsigned characters keep toupper valid for all input byte values.
    std::transform(number.begin(), number.end(), number.begin(),
                   [](unsigned char value)
                   {
                       return static_cast<char>(std::toupper(value));
                   });
    return number;
}

class BinarySearchTree
{
private:
    struct Node
    {
        Course course;
        std::unique_ptr<Node> left;
        std::unique_ptr<Node> right;
        explicit Node(const Course &value) : course(value) {}
    };
    std::unique_ptr<Node> root;
    std::size_t count = 0;

    static void collectInOrder(const Node *node,
                               std::vector<const Course *> &courses)
    {
        if (node == nullptr)
        {
            return;
        }
        collectInOrder(node->left.get(), courses);
        courses.push_back(&node->course);
        collectInOrder(node->right.get(), courses);
    }

    static void printInOrder(const Node *node)
    {
        if (node == nullptr)
        {
            return;
        }
        // Left, current, right produces alphanumeric course-number order.
        printInOrder(node->left.get());
        std::cout << node->course.number << ", " << node->course.title << '\n';
        printInOrder(node->right.get());
    }

    bool validateNode(const Node *node) const
    {
        if (node == nullptr)
        {
            return true;
        }
        for (const auto &prerequisite : node->course.prerequisites)
        {
            if (find(prerequisite) == nullptr)
            {
                std::cout << "File format error: " << node->course.number
                          << " references missing prerequisite "
                          << prerequisite << ".\n";
                return false;
            }
        }
        return validateNode(node->left.get()) && validateNode(node->right.get());
    }

public:
    // Owning pointers release nodes automatically when the tree is destroyed.
    bool insert(const Course &course)
    {
        std::unique_ptr<Node> *link = &root;
        while (*link != nullptr)
        {
            if (course.number == (*link)->course.number)
            {
                return false;
            }
            // Follow the owning link to attach the new node without copying.
            link = course.number < (*link)->course.number
                       ? &(*link)->left
                       : &(*link)->right;
        }
        *link = std::make_unique<Node>(course);
        ++count;
        return true;
    }

    const Course *find(const std::string &number) const
    {
        const Node *current = root.get();
        while (current != nullptr)
        {
            if (number == current->course.number)
            {
                return &current->course;
            }
            current = number < current->course.number
                          ? current->left.get()
                          : current->right.get();
        }
        return nullptr;
    }

    bool validatePrerequisites() const
    {
        return validateNode(root.get());
    }

    std::size_t size() const
    {
        return count;
    }

    void swap(BinarySearchTree &other) noexcept
    {
        root.swap(other.root);
        std::swap(count, other.count);
    }

    void printList() const
    {
        std::cout << "\nHere is a sample schedule:\n\n";
        printInOrder(root.get());
    }

    std::vector<const Course *> completionSchedule() const
    {
        std::vector<const Course *> courses;
        collectInOrder(root.get(), courses);
        std::unordered_map<std::string, std::size_t> indices;
        for (std::size_t index = 0; index < courses.size(); ++index)
        {
            indices.emplace(courses[index]->number, index);
        }

        std::vector<std::size_t> unmet(courses.size(), 0);
        std::vector<std::vector<std::size_t>> dependents(courses.size());
        for (std::size_t index = 0; index < courses.size(); ++index)
        {
            for (const auto &prerequisite : courses[index]->prerequisites)
            {
                const auto found = indices.find(prerequisite);
                if (found == indices.end())
                {
                    return {};
                }
                // Edges point from prerequisites to the courses they unlock.
                dependents[found->second].push_back(index);
                ++unmet[index];
            }
        }

        std::queue<std::size_t> available;
        for (std::size_t index = 0; index < courses.size(); ++index)
        {
            if (unmet[index] == 0)
            {
                available.push(index);
            }
        }

        std::vector<const Course *> schedule;
        while (!available.empty())
        {
            const std::size_t index = available.front();
            available.pop();
            schedule.push_back(courses[index]);
            for (const std::size_t dependent : dependents[index])
            {
                if (--unmet[dependent] == 0)
                {
                    available.push(dependent);
                }
            }
        }

        // Discard partial results when a cycle prevents completing all courses.
        if (schedule.size() != courses.size())
        {
            return {};
        }
        return schedule;
    }

    void printCompletionSchedule() const
    {
        const auto schedule = completionSchedule();
        if (schedule.empty())
        {
            std::cout << "\nCompletion schedule: []\n"
                      << "No complete schedule is possible. "
                      << "Check for prerequisite cycles.\n";
            return;
        }
        std::cout << "\nPrerequisite-valid completion order:\n\n";
        for (const Course *course : schedule)
        {
            std::cout << course->number << ", " << course->title << '\n';
        }
    }

    void printCourse(const std::string &number) const
    {
        const Course *course = find(number);
        if (course == nullptr)
        {
            std::cout << "Course not found: " << number << ".\n";
            return;
        }
        std::cout << '\n'
                  << course->number << ", " << course->title << '\n';
        if (course->prerequisites.empty())
        {
            std::cout << "Prerequisites: None\n";
            return;
        }
        std::cout << "Prerequisites:\n";
        for (const auto &number : course->prerequisites)
        {
            const Course *prerequisite = find(number);
            // Loading validates references; retain a defensive fallback.
            if (prerequisite != nullptr)
            {
                std::cout << "  " << prerequisite->number << ", "
                          << prerequisite->title << '\n';
            }
            else
            {
                std::cout << "  " << number << " (information unavailable)\n";
            }
        }
    }
};

bool parseCourse(const std::string &line, Course &course, std::string &error)
{
    std::stringstream stream(line);
    std::vector<std::string> fields;
    std::string field;
    while (std::getline(stream, field, ','))
    {
        fields.push_back(trim(field));
    }
    if (fields.size() < 2 || fields[0].empty() || fields[1].empty())
    {
        error = "A course requires a nonempty number and title.";
        return false;
    }
    course = Course{};
    course.number = normalizeNumber(fields[0]);
    course.title = fields[1];
    for (std::size_t index = 2; index < fields.size(); ++index)
    {
        const std::string prerequisite = normalizeNumber(fields[index]);
        // The provided CSV pads unused prerequisite columns with empty fields.
        if (prerequisite.empty())
        {
            continue;
        }
        // Cycles, including self-references, are handled by the bonus scheduler.
        course.prerequisites.push_back(prerequisite);
    }
    return true;
}

bool loadCourses(const std::string &filename, BinarySearchTree &courses)
{
    std::ifstream file(filename);
    if (!file.is_open())
    {
        std::cout << "Error: could not open \"" << filename << "\".\n";
        return false;
    }
    // Commit the replacement only after the complete file passes validation.
    BinarySearchTree candidate;
    std::string line;
    std::size_t lineNumber = 0;
    while (std::getline(file, line))
    {
        ++lineNumber;
        // Accept an optional UTF-8 byte order mark at the start of the file.
        if (lineNumber == 1 && line.compare(0, 3, "\xEF\xBB\xBF") == 0)
        {
            line.erase(0, 3);
        }
        line = trim(line);
        if (line.empty())
        {
            continue;
        }
        Course course;
        std::string error;
        if (!parseCourse(line, course, error))
        {
            std::cout << "File format error on line " << lineNumber
                      << ": " << error << '\n';
            return false;
        }
        if (!candidate.insert(course))
        {
            std::cout << "File format error on line " << lineNumber
                      << ": duplicate course " << course.number << ".\n";
            return false;
        }
    }
    if (file.bad() || (file.fail() && !file.eof()))
    {
        std::cout << "Error: the file could not be read completely.\n";
        return false;
    }
    if (candidate.size() == 0)
    {
        std::cout << "Error: the file contains no course records.\n";
        return false;
    }
    // Forward references are valid, so check them after all records are loaded.
    if (!candidate.validatePrerequisites())
    {
        return false;
    }
    courses.swap(candidate);
    std::cout << courses.size() << " courses loaded successfully.\n";
    return true;
}

int main()
{
    BinarySearchTree courses;
    bool dataLoaded = false;
    std::string choice;
    std::cout << "Welcome to the course planner.\n";
    while (true)
    {
        std::cout << "\n1. Load Data Structure.\n2. Print Course List.\n"
                  << "3. Print Course.\n"
                  << "4. Print Prerequisite-Valid Schedule (Bonus).\n"
                  << "9. Exit\n\n"
                  << "What would you like to do? ";
        // Line-based input avoids a failed numeric stream on text menu input.
        if (!std::getline(std::cin, choice))
        {
            break;
        }
        choice = trim(choice);
        if (choice == "9")
        {
            break;
        }
        if (choice == "1")
        {
            std::cout << "Enter the course data file name: ";
            std::string filename;
            if (!std::getline(std::cin, filename))
            {
                break;
            }
            filename = trim(filename);
            if (filename.empty())
            {
                std::cout << "Error: file name cannot be empty.\n";
            }
            else if (loadCourses(filename, courses))
            {
                dataLoaded = true;
            }
            else
            {
                std::cout << "New course data was not loaded.\n";
            }
        }
        else if (choice == "2" || choice == "3" || choice == "4")
        {
            if (!dataLoaded)
            {
                std::cout << "Please load course data first.\n";
                continue;
            }
            if (choice == "2")
            {
                courses.printList();
            }
            else if (choice == "4")
            {
                courses.printCompletionSchedule();
            }
            else
            {
                std::cout << "What course do you want to know about? ";
                std::string number;
                if (!std::getline(std::cin, number))
                {
                    break;
                }
                number = normalizeNumber(number);
                if (number.empty())
                {
                    std::cout << "Error: course number cannot be empty.\n";
                }
                else
                {
                    courses.printCourse(number);
                }
            }
        }
        else
        {
            std::cout << choice << " is not a valid option.\n";
        }
    }
    std::cout << "Thank you for using the course planner!\n";
    return 0;
}
