// CC105 Terminal Assessment 2
// Campus IT Help Desk Ticket Manager
// Build: g++ -std=c++17 -Wall -Wextra -pedantic DynamicHelpDeskTicketManager.cpp -o helpdesk

#include <chrono>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <limits>
#include <string>
#include <utility>

using std::cin;
using std::cout;
using std::endl;
using std::getline;
using std::string;

enum class Priority { Critical = 1, High = 2, Normal = 3, Low = 4 };

struct Ticket {
    string id;
    string requester;
    string category;
    Priority priority = Priority::Normal;
    string status = "Open";
};

struct SortMetrics {
    long long comparisons = 0;
    long long swapsOrMoves = 0;
    double milliseconds = 0.0;
};

string priorityToString(Priority priority) {
    switch (priority) {
        case Priority::Critical: return "Critical";
        case Priority::High: return "High";
        case Priority::Normal: return "Normal";
        case Priority::Low: return "Low";
    }
    return "Unknown";
}

void printDivider() {
    cout << "\n------------------------------------------------------------\n";
}

void printTicket(const Ticket& ticket) {
    cout << "ID: " << ticket.id
         << " | Requester: " << ticket.requester
         << " | Category: " << ticket.category
         << " | Priority: " << priorityToString(ticket.priority)
         << " | Status: " << ticket.status << '\n';
}

string readRequiredLine(const string& prompt) {
    while (true) {
        cout << prompt;
        string value;
        getline(cin, value);
        if (!value.empty()) return value;
        cout << "Input cannot be empty. Please try again.\n";
    }
}

string readOptionalLine(const string& prompt, const string& currentValue) {
    cout << prompt << " [current: " << currentValue << ", Enter to keep]: ";
    string value;
    getline(cin, value);
    return value.empty() ? currentValue : value;
}

int readIntInRange(const string& prompt, int minimum, int maximum) {
    while (true) {
        cout << prompt;
        string input;
        getline(cin, input);
        try {
            size_t consumed = 0;
            int value = std::stoi(input, &consumed);
            if (consumed == input.size() && value >= minimum && value <= maximum) {
                return value;
            }
        } catch (const std::exception&) {
            // The error message below gives the user a safe retry.
        }
        cout << "Invalid choice. Enter a number from " << minimum << " to " << maximum << ".\n";
    }
}

Priority readPriority(const string& prompt = "Priority (1 Critical, 2 High, 3 Normal, 4 Low): ") {
    return static_cast<Priority>(readIntInRange(prompt, 1, 4));
}

// ------------------------- Doubly Linked List -------------------------

struct TicketNode {
    Ticket data;
    TicketNode* prev = nullptr;
    TicketNode* next = nullptr;

    explicit TicketNode(const Ticket& ticket) : data(ticket) {}
};

class TicketList {
private:
    TicketNode* head = nullptr;
    TicketNode* tail = nullptr;
    int count = 0;

public:
    TicketList() = default;
    ~TicketList() { clear(); }

    TicketList(const TicketList&) = delete;
    TicketList& operator=(const TicketList&) = delete;

    bool isEmpty() const { return head == nullptr; }
    int size() const { return count; }

    TicketNode* findById(const string& id) const {
        TicketNode* current = head;
        while (current != nullptr) {
            if (current->data.id == id) return current;
            current = current->next;
        }
        return nullptr;
    }

    bool append(const Ticket& ticket) {
        if (findById(ticket.id) != nullptr) return false;

        TicketNode* node = new TicketNode(ticket);
        if (isEmpty()) {
            head = tail = node;
        } else {
            tail->next = node;
            node->prev = tail;
            tail = node;
        }
        ++count;
        return true;
    }

    bool removeById(const string& id, Ticket& removedTicket) {
        TicketNode* node = findById(id);
        if (node == nullptr) return false;

        removedTicket = node->data;
        if (node->prev != nullptr) node->prev->next = node->next;
        else head = node->next;

        if (node->next != nullptr) node->next->prev = node->prev;
        else tail = node->prev;

        delete node;
        --count;
        return true;
    }

    bool restoreTicket(const Ticket& ticket) {
        TicketNode* node = findById(ticket.id);
        if (node == nullptr) return false;
        node->data = ticket;
        return true;
    }

    void display() const {
        printDivider();
        if (isEmpty()) {
            cout << "No tickets in the master list.\n";
            return;
        }
        cout << "MASTER TICKET LIST (Doubly Linked List)\n";
        TicketNode* current = head;
        while (current != nullptr) {
            printTicket(current->data);
            current = current->next;
        }
        cout << "Total tickets: " << count << '\n';
    }

    Ticket* createSnapshot() const {
        if (count == 0) return nullptr;
        Ticket* snapshot = new Ticket[count];
        TicketNode* current = head;
        int index = 0;
        while (current != nullptr) {
            snapshot[index++] = current->data;
            current = current->next;
        }
        return snapshot;
    }

    void clear() {
        TicketNode* current = head;
        while (current != nullptr) {
            TicketNode* nextNode = current->next;
            delete current;
            current = nextNode;
        }
        head = tail = nullptr;
        count = 0;
    }
};

// ----------------------------- Stack ---------------------------------

enum class ActionType { Add, Update, Delete };

struct UndoAction {
    ActionType type;
    Ticket before;
    Ticket after;
};

struct UndoNode {
    UndoAction action;
    UndoNode* next = nullptr;

    explicit UndoNode(const UndoAction& value) : action(value) {}
};

class ActionStack {
private:
    UndoNode* top = nullptr;

public:
    ActionStack() = default;
    ~ActionStack() { clear(); }

    ActionStack(const ActionStack&) = delete;
    ActionStack& operator=(const ActionStack&) = delete;

    bool isEmpty() const { return top == nullptr; }

    void push(const UndoAction& action) {
        UndoNode* node = new UndoNode(action);
        node->next = top;
        top = node;
    }

    bool pop(UndoAction& action) {
        if (isEmpty()) return false;
        UndoNode* oldTop = top;
        action = oldTop->action;
        top = top->next;
        delete oldTop;
        return true;
    }

    void clear() {
        UndoNode* current = top;
        while (current != nullptr) {
            UndoNode* nextNode = current->next;
            delete current;
            current = nextNode;
        }
        top = nullptr;
    }
};

// ------------------------- Linked Priority Queue ---------------------

struct QueueNode {
    Ticket ticket;
    QueueNode* next = nullptr;

    explicit QueueNode(const Ticket& value) : ticket(value) {}
};

class PriorityQueue {
private:
    QueueNode* front = nullptr;
    QueueNode* rear = nullptr;

public:
    PriorityQueue() = default;
    ~PriorityQueue() { clear(); }

    PriorityQueue(const PriorityQueue&) = delete;
    PriorityQueue& operator=(const PriorityQueue&) = delete;

    bool isEmpty() const { return front == nullptr; }

    void enqueue(const Ticket& ticket) {
        QueueNode* node = new QueueNode(ticket);
        const int newPriority = static_cast<int>(ticket.priority);

        if (isEmpty()) {
            front = rear = node;
            return;
        }

        if (newPriority < static_cast<int>(front->ticket.priority)) {
            node->next = front;
            front = node;
            return;
        }

        QueueNode* current = front;
        // <= preserves FIFO order for tickets with the same priority.
        while (current->next != nullptr &&
               static_cast<int>(current->next->ticket.priority) <= newPriority) {
            current = current->next;
        }
        node->next = current->next;
        current->next = node;
        if (node->next == nullptr) rear = node;
    }

    bool dequeue(Ticket& ticket) {
        if (isEmpty()) return false;
        QueueNode* oldFront = front;
        ticket = oldFront->ticket;
        front = front->next;
        if (front == nullptr) rear = nullptr;
        delete oldFront;
        return true;
    }

    void display() const {
        printDivider();
        if (isEmpty()) {
            cout << "Dispatch queue is empty.\n";
            return;
        }
        cout << "DISPATCH QUEUE (Priority first, FIFO for ties)\n";
        QueueNode* current = front;
        int position = 1;
        while (current != nullptr) {
            cout << position++ << ". ";
            printTicket(current->ticket);
            current = current->next;
        }
    }

    void clear() {
        QueueNode* current = front;
        while (current != nullptr) {
            QueueNode* nextNode = current->next;
            delete current;
            current = nextNode;
        }
        front = rear = nullptr;
    }
};

// ------------------------- Sorting and Search ------------------------

template <typename T, typename GreaterThan>
SortMetrics bubbleSort(T* items, int count, GreaterThan greaterThan) {
    SortMetrics metrics;
    for (int end = count - 1; end > 0; --end) {
        bool changed = false;
        for (int i = 0; i < end; ++i) {
            ++metrics.comparisons;
            if (greaterThan(items[i], items[i + 1])) {
                std::swap(items[i], items[i + 1]);
                ++metrics.swapsOrMoves;
                changed = true;
            }
        }
        if (!changed) break;
    }
    return metrics;
}

template <typename T, typename GreaterThan>
SortMetrics selectionSort(T* items, int count, GreaterThan greaterThan) {
    SortMetrics metrics;
    for (int i = 0; i < count - 1; ++i) {
        int minimum = i;
        for (int j = i + 1; j < count; ++j) {
            ++metrics.comparisons;
            if (greaterThan(items[minimum], items[j])) minimum = j;
        }
        if (minimum != i) {
            std::swap(items[i], items[minimum]);
            ++metrics.swapsOrMoves;
        }
    }
    return metrics;
}

template <typename T, typename GreaterThan>
SortMetrics insertionSort(T* items, int count, GreaterThan greaterThan) {
    SortMetrics metrics;
    for (int i = 1; i < count; ++i) {
        T key = items[i];
        int j = i;
        while (j > 0) {
            ++metrics.comparisons;
            if (!greaterThan(items[j - 1], key)) break;
            items[j] = items[j - 1];
            ++metrics.swapsOrMoves;  // A shift is counted as a move.
            --j;
        }
        items[j] = key;
    }
    return metrics;
}

template <typename T, typename GreaterThan>
SortMetrics runSort(int choice, T* items, int count, GreaterThan greaterThan) {
    if (choice == 1) return bubbleSort(items, count, greaterThan);
    if (choice == 2) return selectionSort(items, count, greaterThan);
    return insertionSort(items, count, greaterThan);
}

string algorithmName(int choice) {
    if (choice == 1) return "Bubble Sort";
    if (choice == 2) return "Selection Sort";
    return "Insertion Sort";
}

void printMetrics(const SortMetrics& metrics) {
    cout << "Comparisons: " << metrics.comparisons
         << " | Swaps/Moves: " << metrics.swapsOrMoves
         << " | Execution Time: " << std::fixed << std::setprecision(4)
         << metrics.milliseconds << " ms\n";
}

void runTicketSort(const TicketList& tickets) {
    if (tickets.isEmpty()) {
        cout << "Add a ticket before sorting.\n";
        return;
    }

    int choice = readIntInRange("Sort (1 Bubble, 2 Selection, 3 Insertion): ", 1, 3);
    int count = tickets.size();
    Ticket* snapshot = tickets.createSnapshot();
    auto greaterById = [](const Ticket& left, const Ticket& right) {
        return left.id > right.id;
    };

    const auto start = std::chrono::high_resolution_clock::now();
    SortMetrics metrics = runSort(choice, snapshot, count, greaterById);
    const auto finish = std::chrono::high_resolution_clock::now();
    metrics.milliseconds = std::chrono::duration<double, std::milli>(finish - start).count();

    printDivider();
    cout << algorithmName(choice) << " RESULT ON TICKET SNAPSHOT\n";
    for (int i = 0; i < count; ++i) printTicket(snapshot[i]);
    printMetrics(metrics);
    delete[] snapshot;
}

void binarySearchTickets(const TicketList& tickets) {
    if (tickets.isEmpty()) {
        cout << "Add a ticket before searching.\n";
        return;
    }

    string id = readRequiredLine("Ticket ID to find: ");
    int count = tickets.size();
    Ticket* snapshot = tickets.createSnapshot();
    auto greaterById = [](const Ticket& left, const Ticket& right) {
        return left.id > right.id;
    };
    SortMetrics setupMetrics = insertionSort(snapshot, count, greaterById);

    int low = 0;
    int high = count - 1;
    int comparisons = 0;
    bool found = false;
    while (low <= high) {
        int middle = low + (high - low) / 2;
        ++comparisons;
        if (snapshot[middle].id == id) {
            found = true;
            printTicket(snapshot[middle]);
            break;
        }
        if (snapshot[middle].id < id) low = middle + 1;
        else high = middle - 1;
    }

    if (!found) cout << "Ticket not found.\n";
    cout << "Binary Search comparisons: " << comparisons
         << " | Setup sort comparisons: " << setupMetrics.comparisons << '\n';
    delete[] snapshot;
}

int* makeBenchmarkData(int count) {
    int* values = new int[count];
    for (int i = 0; i < count; ++i) values[i] = i + 1;

    uint32_t state = static_cast<uint32_t>(105 + count);
    for (int i = count - 1; i > 0; --i) {
        state = state * 1664525u + 1013904223u;
        int j = static_cast<int>(state % static_cast<uint32_t>(i + 1));
        std::swap(values[i], values[j]);
    }
    return values;
}

void benchmarkOneAlgorithm(const int* original, int count, int choice) {
    int* working = new int[count];
    for (int i = 0; i < count; ++i) working[i] = original[i];

    auto greaterInt = [](int left, int right) { return left > right; };
    const auto start = std::chrono::high_resolution_clock::now();
    SortMetrics metrics = runSort(choice, working, count, greaterInt);
    const auto finish = std::chrono::high_resolution_clock::now();
    metrics.milliseconds = std::chrono::duration<double, std::milli>(finish - start).count();

    cout << std::left << std::setw(17) << algorithmName(choice);
    printMetrics(metrics);
    delete[] working;
}

void runBenchmark() {
    int sizeChoice = readIntInRange("Dataset (1 = 10, 2 = 500, 3 = 5000): ", 1, 3);
    int count = sizeChoice == 1 ? 10 : (sizeChoice == 2 ? 500 : 5000);
    int* original = makeBenchmarkData(count);

    printDivider();
    cout << "BENCHMARK DATASET SIZE: " << count << '\n';
    cout << "Algorithm        Metrics\n";
    benchmarkOneAlgorithm(original, count, 1);
    benchmarkOneAlgorithm(original, count, 2);
    benchmarkOneAlgorithm(original, count, 3);
    cout << "Big O note: all three algorithms have O(n^2) average/worst-case behavior.\n";

    delete[] original;
}

// ---------------------------- Menu Actions ---------------------------

Ticket collectNewTicket() {
    Ticket ticket;
    ticket.id = readRequiredLine("Ticket ID (example T-1001): ");
    ticket.requester = readRequiredLine("Requester name: ");
    ticket.category = readRequiredLine("Issue category: ");
    ticket.priority = readPriority();
    ticket.status = "Open";
    return ticket;
}

void addTicket(TicketList& tickets, ActionStack& undoStack) {
    Ticket ticket = collectNewTicket();
    if (!tickets.append(ticket)) {
        cout << "A ticket with that ID already exists.\n";
        return;
    }
    undoStack.push({ActionType::Add, Ticket{}, ticket});
    cout << "Ticket added successfully.\n";
}

void updateTicket(TicketList& tickets, ActionStack& undoStack) {
    string id = readRequiredLine("Ticket ID to update: ");
    TicketNode* node = tickets.findById(id);
    if (node == nullptr) {
        cout << "Ticket not found.\n";
        return;
    }

    Ticket before = node->data;
    node->data.requester = readOptionalLine("Requester", node->data.requester);
    node->data.category = readOptionalLine("Category", node->data.category);
    node->data.status = readOptionalLine("Status", node->data.status);
    int changePriority = readIntInRange("Change priority? (1 Yes, 2 No): ", 1, 2);
    if (changePriority == 1) node->data.priority = readPriority();

    undoStack.push({ActionType::Update, before, node->data});
    cout << "Ticket updated successfully.\n";
}

void deleteTicket(TicketList& tickets, ActionStack& undoStack) {
    string id = readRequiredLine("Ticket ID to delete: ");
    Ticket removed;
    if (!tickets.removeById(id, removed)) {
        cout << "Ticket not found.\n";
        return;
    }
    undoStack.push({ActionType::Delete, removed, Ticket{}});
    cout << "Ticket deleted successfully.\n";
}

void enqueueTicket(const TicketList& tickets, PriorityQueue& queue) {
    string id = readRequiredLine("Ticket ID to place in dispatch queue: ");
    TicketNode* node = tickets.findById(id);
    if (node == nullptr) {
        cout << "Ticket not found.\n";
        return;
    }
    queue.enqueue(node->data);
    cout << "Ticket queued for dispatch.\n";
}

void dequeueTicket(PriorityQueue& queue) {
    Ticket next;
    if (!queue.dequeue(next)) {
        cout << "Queue underflow: no ticket is waiting.\n";
        return;
    }
    cout << "Next ticket dispatched:\n";
    printTicket(next);
}

void undoLastAction(TicketList& tickets, ActionStack& undoStack) {
    UndoAction action;
    if (!undoStack.pop(action)) {
        cout << "Stack underflow: there is no action to undo.\n";
        return;
    }

    bool completed = false;
    if (action.type == ActionType::Add) {
        Ticket removed;
        completed = tickets.removeById(action.after.id, removed);
    } else if (action.type == ActionType::Delete) {
        completed = tickets.append(action.before);
    } else {
        completed = tickets.restoreTicket(action.before);
    }
    cout << (completed ? "Last action undone.\n" : "Undo could not be completed safely.\n");
}

void linearSearch(const TicketList& tickets) {
    string id = readRequiredLine("Ticket ID to find: ");
    TicketNode* node = tickets.findById(id);
    if (node == nullptr) cout << "Ticket not found.\n";
    else printTicket(node->data);
}

void showMenu() {
    printDivider();
    cout << "CAMPUS IT HELP DESK TICKET MANAGER\n"
         << "1. Add ticket\n"
         << "2. Display master ticket list\n"
         << "3. Update ticket\n"
         << "4. Delete ticket\n"
         << "5. Add ticket to priority queue\n"
         << "6. Dispatch next queued ticket\n"
         << "7. Display dispatch queue\n"
         << "8. Undo last CRUD action\n"
         << "9. Linear Search by ticket ID\n"
         << "10. Binary Search by ticket ID\n"
         << "11. Sort ticket snapshot\n"
         << "12. Run benchmark suite\n"
         << "0. Exit\n";
}

int main() {
    TicketList tickets;
    ActionStack undoStack;
    PriorityQueue dispatchQueue;

    cout << "Dynamic Data Engine and Benchmarking Suite\n";
    cout << "Campus IT Help Desk Ticket Manager\n";

    bool running = true;
    while (running) {
        showMenu();
        int choice = readIntInRange("Choose an option: ", 0, 12);
        switch (choice) {
            case 1: addTicket(tickets, undoStack); break;
            case 2: tickets.display(); break;
            case 3: updateTicket(tickets, undoStack); break;
            case 4: deleteTicket(tickets, undoStack); break;
            case 5: enqueueTicket(tickets, dispatchQueue); break;
            case 6: dequeueTicket(dispatchQueue); break;
            case 7: dispatchQueue.display(); break;
            case 8: undoLastAction(tickets, undoStack); break;
            case 9: linearSearch(tickets); break;
            case 10: binarySearchTickets(tickets); break;
            case 11: runTicketSort(tickets); break;
            case 12: runBenchmark(); break;
            case 0: running = false; break;
        }
    }

    cout << "Program closed. Dynamic memory cleanup routines completed.\n";
    return 0;
}
