// =============================================================================
// BEHAVIORAL PATTERNS - C++ Implementations
// Based on: Design Patterns: Elements of Reusable Object-Oriented Software
//           Erich Gamma, Richard Helm, Ralph Johnson, John Vlissides (GoF)
//
// Patterns covered (11):
//   Chain of Responsibility, Command, Interpreter, Iterator, Mediator,
//   Memento, Observer, State, Strategy, Template Method, Visitor
// =============================================================================

#include <algorithm>
#include <functional>
#include <memory>
#include <print>
#include <sstream>
#include <stack>
#include <string>
#include <vector>
#ifdef _WIN32
#include <windows.h>
#endif

/// ─────────────────────────────────────────────────────────────────────────────
/// PATTERN 13: CHAIN OF RESPONSIBILITY
/// ─────────────────────────────────────────────────────────────────────────────
///
/// PROJECT: Customer Support Ticket Escalation
///   Support tickets have severity levels (LOW, MEDIUM, HIGH, CRITICAL).
///   Level-1 agents handle LOW, L2 handles up to MEDIUM, Manager handles HIGH,
///   CTO handles CRITICAL. A ticket bubbles up the chain until handled.
///   Ticket routing rules can change without altering any handler.
///
/// WHY CHAIN OF RESPONSIBILITY?
///   ✓ More than one handler can process the request, and the right one
///     isn't known until runtime.
///   ✓ We want to add/remove handlers (new tier between L2 and Manager) by
///     just rewiring the chain - no handler needs to change.
///   ✓ The sender (ticket submission system) need not know WHO will handle it.
///
/// ALTERNATIVES RULED OUT:
///   Switch/if-else: hard-codes routing; every new tier breaks the code.
///   Command+Registry: overkill; we need sequential search, not lookup.
///
/// KEY LIABILITY: A ticket might fall off the end unhandled. We add a catch-all.
/// ─────────────────────────────────────────────────────────────────────────────
namespace chain_of_responsibility {

enum class Severity { LOW = 1, MEDIUM = 2, HIGH = 3, CRITICAL = 4 };

std::string severityName(Severity sev) {
    switch (sev) {
        case Severity::LOW:
            return "LOW";
        case Severity::MEDIUM:
            return "MEDIUM";
        case Severity::HIGH:
            return "HIGH";
        case Severity::CRITICAL:
            return "CRITICAL";
    }
    return "?";
}

struct Ticket {
    std::string description;
    Severity    severity;
};

struct SupportHandler {
    std::shared_ptr<SupportHandler> successor;

    void setSuccessor(std::shared_ptr<SupportHandler> succ) { successor = succ; }

    virtual void handle(const Ticket& tkt) {
        if (successor)
            successor->handle(tkt);
        else
            std::println("[Unhandled] Ticket dropped: {}", tkt.description);
    }
    virtual ~SupportHandler() = default;
};

struct L1Agent : SupportHandler {
    void handle(const Ticket& tkt) override {
        if (tkt.severity == Severity::LOW) {
            std::println("[L1 Agent] Resolved '{}'", tkt.description);
        } else {
            std::println("[L1 Agent] Escalating {} ticket", severityName(tkt.severity));
            SupportHandler::handle(tkt);
        }
    }
};
struct L2Agent : SupportHandler {
    void handle(const Ticket& tkt) override {
        if (tkt.severity <= Severity::MEDIUM) {
            std::println("[L2 Agent] Resolved '{}'", tkt.description);
        } else {
            std::println("[L2 Agent] Escalating {} ticket", severityName(tkt.severity));
            SupportHandler::handle(tkt);
        }
    }
};
struct Manager : SupportHandler {
    void handle(const Ticket& tkt) override {
        if (tkt.severity <= Severity::HIGH) {
            std::println("[Manager] Resolved '{}'", tkt.description);
        } else {
            std::println("[Manager] Escalating CRITICAL ticket to CTO");
            SupportHandler::handle(tkt);
        }
    }
};
struct CTO : SupportHandler {
    void handle(const Ticket& tkt) override {
        std::println("[CTO] Personally handling CRITICAL: '{}'", tkt.description);
    }
};

void demo() {
    std::println("\n Chain of Responsibility: Support Escalation ");
    auto lvl1 = std::make_shared<L1Agent>();
    auto lvl2 = std::make_shared<L2Agent>();
    auto mgr  = std::make_shared<Manager>();
    auto cto  = std::make_shared<CTO>();
    lvl1->setSuccessor(lvl2);
    lvl2->setSuccessor(mgr);
    mgr->setSuccessor(cto);

    std::vector<Ticket> tickets = {
        {"Password reset", Severity::LOW},
        {"Billing discrepancy", Severity::MEDIUM},
        {"Data loss on save", Severity::HIGH},
        {"Production database down", Severity::CRITICAL},
    };
    for (const auto& tkt : tickets) {
        std::println("Submitting [{}]: {}", severityName(tkt.severity), tkt.description);
        lvl1->handle(tkt);
    }
}

}  // namespace chain_of_responsibility

/// ─────────────────────────────────────────────────────────────────────────────
/// PATTERN 14: COMMAND
/// ─────────────────────────────────────────────────────────────────────────────
///
/// PROJECT: Smart Bulb Remote with Undo
///   A smart home remote has buttons that can turn a bulb on/off, or dim it.
///   Each button action must be undoable. Actions should be loggable and
///   potentially scriptable. The remote (Invoker) shouldn't know how bulbs work.
///
/// WHY COMMAND?
///   ✓ Decouples the remote (Invoker) from the bulb (Receiver).
///   ✓ Undo: each Command stores enough state to reverse itself.
///   ✓ Commands are first-class objects: they can be stored in a history list,
///     logged to disk, or composed into MacroCommands.
///
/// ALTERNATIVES RULED OUT:
///   Direct method calls: the remote directly calls bulb.on(); can't undo,
///                         can't log, can't queue for later execution.
///   Lambda callbacks: simpler in modern C++ but can't implement Undo()
///                         without extra state capture; Command is cleaner.
/// ─────────────────────────────────────────────────────────────────────────────
namespace command {

/// Receiver: knows how to perform the actual work
struct SmartBulb {
    bool isOn       = false;
    int  brightness = 100;

    void turnOn() {
        isOn = true;
        std::println("[Bulb] ON  (brightness={})", brightness);
    }
    void turnOff() {
        isOn = false;
        std::println("[Bulb] OFF");
    }
    void dim(int brt) {
        brightness = brt;
        std::println("[Bulb] Dimmed to {}%", brt);
    }
};

/// Command interface
struct Command {
    virtual void execute() = 0;
    virtual void undo()    = 0;
    virtual ~Command()     = default;
};

/// Concrete Commands: each binds a Receiver with specific parameters
struct TurnOnCommand : Command {
    SmartBulb& bulb;
    bool       wasOn;
    explicit TurnOnCommand(SmartBulb& blb) : bulb(blb), wasOn(false) {}
    void execute() override {
        wasOn = bulb.isOn;
        bulb.turnOn();
    }
    void undo() override {
        if (!wasOn)
            bulb.turnOff();
    }
};
struct TurnOffCommand : Command {
    SmartBulb& bulb;
    bool       wasOn;
    explicit TurnOffCommand(SmartBulb& blb) : bulb(blb), wasOn(false) {}
    void execute() override {
        wasOn = bulb.isOn;
        bulb.turnOff();
    }
    void undo() override {
        if (wasOn)
            bulb.turnOn();
    }
};
struct DimCommand : Command {
    SmartBulb& bulb;
    int        newBrightness, prevBrightness;
    DimCommand(SmartBulb& blb, int level) : bulb(blb), newBrightness(level), prevBrightness(100) {}
    void execute() override {
        prevBrightness = bulb.brightness;
        bulb.dim(newBrightness);
    }
    void undo() override { bulb.dim(prevBrightness); }
};

/// MacroCommand: Composite of Commands (executes/undoes several in sequence)
struct MacroCommand : Command {
    std::vector<std::unique_ptr<Command>> commands;
    void add(std::unique_ptr<Command> cmd) { commands.push_back(std::move(cmd)); }
    void execute() override {
        for (auto& cmd : commands) cmd->execute();
    }
    void undo() override {
        for (auto itr = commands.rbegin(); itr != commands.rend(); ++itr) (*itr)->undo();
    }
};

/// Invoker: stores and executes commands; maintains undo history
struct Remote {
    std::stack<Command*> history;

    void press(Command& cmd) {
        cmd.execute();
        history.push(&cmd);
    }
    void pressUndo() {
        if (!history.empty()) {
            history.top()->undo();
            history.pop();
        } else {
            std::println("[Remote] Nothing to undo");
        }
    }
};

void demo() {
    std::println("\n Command: Smart Bulb Remote with Undo ");
    SmartBulb bulb;
    Remote    remote;

    TurnOnCommand  onCmd(bulb);
    DimCommand     dim(bulb, 40);
    TurnOffCommand off(bulb);

    std::println("Press ON:");
    remote.press(onCmd);
    std::println("Press DIM 40:");
    remote.press(dim);
    std::println("UNDO dim:");
    remote.pressUndo();
    std::println("UNDO on:");
    remote.pressUndo();
    std::println("UNDO (empty):");
    remote.pressUndo();
}

}  // namespace command

/// ─────────────────────────────────────────────────────────────────────────────
/// PATTERN 15: INTERPRETER
/// ─────────────────────────────────────────────────────────────────────────────
///
/// PROJECT: Simple Arithmetic Expression Evaluator
///   A configuration or rule engine needs to evaluate expressions like
///   "3 + 4 - 2". The grammar has 3 rules: Number, Add, Subtract.
///   Each rule becomes a class; the expression is parsed into an AST,
///   then evaluated recursively via Interpret().
///
/// WHY INTERPRETER?
///   ✓ The grammar is very simple (<= 5 rules: Number, Add, Subtract).
///   ✓ We want to add new operations (e.g., Multiply) by adding one class.
///   ✓ The AST = Composite, so Interpreter extends Composite naturally.
///
/// WHEN NOT TO USE:
///   Complex grammars (10+ rules): use ANTLR or a proper parser generator.
///   Hot evaluation path: compile to bytecode instead.
/// ─────────────────────────────────────────────────────────────────────────────
namespace interpreter {

/// AbstractExpression
struct Expr {
    virtual int         interpret() const = 0;
    virtual std::string str() const       = 0;
    virtual ~Expr()                       = default;
};

/// TerminalExpression: a literal number
struct NumberExpr : Expr {
    int value;
    explicit NumberExpr(int val) : value(val) {}
    int         interpret() const override { return value; }
    std::string str() const override { return std::to_string(value); }
};

/// NonTerminalExpression: Add
struct AddExpr : Expr {
    std::unique_ptr<Expr> left, right;
    AddExpr(std::unique_ptr<Expr> lhs, std::unique_ptr<Expr> rhs)
        : left(std::move(lhs)), right(std::move(rhs)) {}
    int         interpret() const override { return left->interpret() + right->interpret(); }
    std::string str() const override { return "(" + left->str() + " + " + right->str() + ")"; }
};

/// NonTerminalExpression: Subtract
struct SubExpr : Expr {
    std::unique_ptr<Expr> left, right;
    SubExpr(std::unique_ptr<Expr> lhs, std::unique_ptr<Expr> rhs)
        : left(std::move(lhs)), right(std::move(rhs)) {}
    int         interpret() const override { return left->interpret() - right->interpret(); }
    std::string str() const override { return "(" + left->str() + " - " + right->str() + ")"; }
};

void demo() {
    std::println("\n Interpreter: Arithmetic Expression Evaluator ");

    // Build AST for: ((3 + 4) - 2)
    auto expr = std::make_unique<SubExpr>(
        std::make_unique<AddExpr>(std::make_unique<NumberExpr>(3), std::make_unique<NumberExpr>(4)),
        std::make_unique<NumberExpr>(2)
    );

    std::println("Expression: {}", expr->str());
    std::println("Result:     {}", expr->interpret());

    // ((10 - 3) + (2 + 1)) = 10
    auto expr2 = std::make_unique<AddExpr>(
        std::make_unique<SubExpr>(
            std::make_unique<NumberExpr>(10), std::make_unique<NumberExpr>(3)
        ),
        std::make_unique<AddExpr>(std::make_unique<NumberExpr>(2), std::make_unique<NumberExpr>(1))
    );
    std::println("Expression: {} = {}", expr2->str(), expr2->interpret());
}

}  // namespace interpreter

/// ─────────────────────────────────────────────────────────────────────────────
/// PATTERN 16: ITERATOR
/// ─────────────────────────────────────────────────────────────────────────────
///
/// PROJECT: Music Playlist with Multiple Traversal Modes
///   A Playlist stores songs. Users want to traverse it in two ways: forward
///   (normal play) and shuffled (random play). Multiple iterators can run
///   simultaneously without interfering. The Playlist doesn't expose its
///   internal vector directly.
///
/// WHY ITERATOR?
///   ✓ The aggregate (Playlist) must not expose its internal representation.
///   ✓ We need multiple simultaneous traversal strategies (forward, shuffle).
///   ✓ Uniform interface - client code works with any Iterator.
///
/// NOTE: Modern C++ has range-for and std::begin/end, which embed this pattern.
///   We implement it from scratch to understand the structure. In real code,
///   you'd expose begin()/end() and implement operator++ for range-for support.
/// ─────────────────────────────────────────────────────────────────────────────
namespace iterator {

struct Song {
    std::string title, artist;
};

/// Iterator interface
struct PlaylistIterator {
    virtual bool hasNext() const = 0;
    virtual Song next()          = 0;
    virtual ~PlaylistIterator()  = default;
};

/// Aggregate
struct Playlist {
    std::vector<Song> songs;

    void addSong(std::string title, std::string artist) {
        songs.push_back({std::move(title), std::move(artist)});
    }

    /// Factory Method: creates the right iterator
    std::unique_ptr<PlaylistIterator> forwardIterator() const;
    std::unique_ptr<PlaylistIterator> shuffleIterator() const;
};

/// Concrete Iterator: forward
struct ForwardIterator : PlaylistIterator {
    const std::vector<Song>& songs;
    size_t                   index = 0;
    explicit ForwardIterator(const std::vector<Song>& src) : songs(src) {}
    bool hasNext() const override { return index < songs.size(); }
    Song next() override { return songs[index++]; }
};

/// Concrete Iterator: shuffle (Fisher-Yates at creation)
struct ShuffleIterator : PlaylistIterator {
    std::vector<Song> shuffled;
    size_t            index = 0;
    explicit ShuffleIterator(std::vector<Song> src) : shuffled(std::move(src)) {
        // Simple deterministic shuffle for demo
        for (int idx = static_cast<int>(shuffled.size()) - 1; idx > 0; --idx)
            std::swap(shuffled[idx], shuffled[(idx * 7 + 3) % (idx + 1)]);
    }
    bool hasNext() const override { return index < shuffled.size(); }
    Song next() override { return shuffled[index++]; }
};

std::unique_ptr<PlaylistIterator> Playlist::forwardIterator() const {
    return std::make_unique<ForwardIterator>(songs);
}
std::unique_ptr<PlaylistIterator> Playlist::shuffleIterator() const {
    return std::make_unique<ShuffleIterator>(songs);
}

void play(PlaylistIterator& itr, const std::string& mode) {
    std::println("[{} mode]", mode);
    while (itr.hasNext()) {
        auto sng = itr.next();
        std::println("♪ {} - {}", sng.title, sng.artist);
    }
}

void demo() {
    std::println("\n Iterator: Music Playlist Traversal ");
    Playlist playlist;
    playlist.addSong("Bohemian Rhapsody", "Queen");
    playlist.addSong("Hotel California", "Eagles");
    playlist.addSong("Stairway to Heaven", "Led Zeppelin");
    playlist.addSong("Imagine", "John Lennon");

    auto fwd = playlist.forwardIterator();
    play(*fwd, "Forward");

    auto shuf = playlist.shuffleIterator();
    play(*shuf, "Shuffle");
}

}  // namespace iterator

/// ─────────────────────────────────────────────────────────────────────────────
/// PATTERN 17: MEDIATOR
/// ─────────────────────────────────────────────────────────────────────────────
///
/// PROJECT: Chat Room
///   Users in a chat room need to send messages to each other. Without a
///   Mediator, User A would hold a reference to User B, C, D… and vice versa -
///   a dense many-to-many mesh that makes users tightly coupled to each other.
///   The ChatRoom (Mediator) centralizes all communication.
///
/// WHY MEDIATOR?
///   ✓ N users would otherwise have N*(N-1) references to each other.
///   ✓ Each user knows only the ChatRoom - you can add a new user or change
///     broadcast logic without touching any User class.
///   ✓ The Mediator makes communication explicit and centralizable.
///
/// MEDIATOR vs OBSERVER:
///   Observer: Subject broadcasts to all Observers (one-to-many).
///   Mediator: Any colleague can trigger coordination (many-to-many → one hub).
///   They're often used together: Colleagues use Observer to notify the
///   Mediator.
/// ─────────────────────────────────────────────────────────────────────────────
namespace mediator {

struct User;  // forward declaration

/// Mediator interface
struct ChatRoom {
    virtual void sendMessage(const std::string& msg, User* sender) = 0;
    virtual void addUser(User* usr)                                = 0;
    virtual ~ChatRoom()                                            = default;
};

/// Colleague: knows only the Mediator
struct User {
    std::string name;
    ChatRoom*   room = nullptr;

    explicit User(std::string nam) : name(std::move(nam)) {}

    void join(ChatRoom& roo) {
        room = &roo;
        roo.addUser(this);
    }

    void send(const std::string& msg) {
        std::println("[{}] sends: {}", name, msg);
        if (room)
            room->sendMessage(msg, this);
    }
    void receive(const std::string& msg, const std::string& from) {
        std::println("-> [{}] receives from {}: {}", name, from, msg);
    }
};

/// Concrete Mediator
struct GeneralChatRoom : ChatRoom {
    std::vector<User*> users;

    void addUser(User* usr) override { users.push_back(usr); }

    void sendMessage(const std::string& msg, User* sender) override {
        for (User* usr : users)
            if (usr != sender)
                usr->receive(msg, sender->name);
    }
};

void demo() {
    std::println("\n Mediator: Chat Room ");
    GeneralChatRoom room;
    User            alice("Alice"), bob("Bob"), carol("Carol");
    alice.join(room);
    bob.join(room);
    carol.join(room);

    alice.send("Hey everyone!");
    bob.send("Hi Alice and Carol!");
}

}  // namespace mediator

/// ─────────────────────────────────────────────────────────────────────────────
/// PATTERN 18: MEMENTO
/// ─────────────────────────────────────────────────────────────────────────────
///
/// PROJECT: Text Editor with Undo/Redo
///   A text editor lets users type and delete text. The user must be able to
///   undo and redo changes. The editor's internal state (the text buffer) must
///   be snapshotted without exposing its internals to the undo manager.
///
/// WHY MEMENTO?
///   ✓ We need to capture and externalize the Editor's state (the text buffer).
///   ✓ Encapsulation must be preserved - the Caretaker (History) must NOT be
///     able to inspect or modify the stored state.
///   ✓ This separates "what to store" (Originator's responsibility) from
///     "when/how long to store it" (Caretaker's responsibility).
///
/// C++ IMPLEMENTATION NOTE:
///   We use an inner class (Memento nested in TextEditor) to enforce the
///   wide-interface (only Editor reads it) vs narrow-interface (History just
///   holds an opaque object) distinction without C++ friend gymnastics.
/// ─────────────────────────────────────────────────────────────────────────────
namespace memento {

/// Originator
class TextEditor {
   public:
    /// Memento: opaque snapshot (only Originator accesses internals)
    struct Snapshot {
        friend class TextEditor;

       private:
        std::string buffer;
        explicit Snapshot(std::string buf) : buffer(std::move(buf)) {}
    };

    void type(const std::string& text) {
        m_buffer += text;
        std::println("Typed: \"{}\" → buffer: \"{}\"", text, m_buffer);
    }
    void deleteLast(size_t num) {
        // Adding parentheses around std::min prevents macro replacement
        num = (std::min)(num, m_buffer.size());
        m_buffer.erase(m_buffer.size() - num);
        std::println("Deleted {} chars → buffer: \"{}\"", num, m_buffer);
    }

    Snapshot save() const {
        std::println("[Editor] Snapshot saved");
        return Snapshot(m_buffer);
    }
    void restore(const Snapshot& snap) {
        m_buffer = snap.buffer;
        std::println("[Editor] Restored → buffer: \"{}\"", m_buffer);
    }
    const std::string& text() const { return m_buffer; }

   private:
    std::string m_buffer;
};

/// Caretaker: holds snapshots; can't read their contents
struct History {
    std::stack<TextEditor::Snapshot> undoStack;
    std::stack<TextEditor::Snapshot> redoStack;

    void save(TextEditor& edt) {
        undoStack.push(edt.save());
        while (!redoStack.empty()) redoStack.pop();  // clear redo on new edit
    }
    void undo(TextEditor& edt) {
        if (undoStack.empty()) {
            std::println("[History] Nothing to undo");
            return;
        }
        redoStack.push(edt.save());  // save current state for redo
        edt.restore(undoStack.top());
        undoStack.pop();
    }
    void redo(TextEditor& edt) {
        if (redoStack.empty()) {
            std::println("[History] Nothing to redo");
            return;
        }
        undoStack.push(edt.save());
        edt.restore(redoStack.top());
        redoStack.pop();
    }
};

void demo() {
    std::println("\n Memento: Text Editor Undo/Redo ");
    TextEditor editor;
    History    history;

    history.save(editor);
    editor.type("Hello");
    history.save(editor);

    editor.type(", World");
    history.save(editor);

    editor.deleteLast(5);
    history.save(editor);

    std::println("Undo:");
    history.undo(editor);
    std::println("Undo:");
    history.undo(editor);
    std::println("Redo:");
    history.redo(editor);
}

}  // namespace memento

/// ─────────────────────────────────────────────────────────────────────────────
/// PATTERN 19: OBSERVER
/// ─────────────────────────────────────────────────────────────────────────────
///
/// PROJECT: Stock Price Tracker
///   A stock exchange publishes price updates for stocks. Investors (Observers)
///   register to be notified when a stock they follow changes price. The
///   exchange (Subject) doesn't know how many investors exist or what they do
///   with the data.
///
/// WHY OBSERVER?
///   ✓ A change to the Subject (StockExchange) must automatically notify an
///     unknown, dynamic number of dependents (Investors).
///   ✓ The Subject and Observers should be loosely coupled - investors can be
///     added/removed at runtime without changing the exchange.
///   ✓ This is the textbook publish-subscribe problem.
///
/// PULL vs PUSH:
///   PUSH model shown here for clarity, but GoF recommends PULL (Subject sends
///   minimal notification; Observers query for specific data they need) because
///   it's more general and avoids sending data Observers don't care about.
/// ─────────────────────────────────────────────────────────────────────────────
namespace observer {

struct StockObserver {
    virtual void onPriceChange(const std::string& ticker, double price) = 0;
    virtual ~StockObserver()                                            = default;
};

/// Subject
struct StockExchange {
    std::vector<StockObserver*>             observers;
    std::unordered_map<std::string, double> prices;

    void attach(StockObserver* obs) { observers.push_back(obs); }
    void detach(StockObserver* obs) {
        observers.erase(std::remove(observers.begin(), observers.end(), obs), observers.end());
    }

    void setPrice(const std::string& ticker, double price) {
        prices[ticker] = price;
        std::println("[Exchange] {} → ${}", ticker, price);
        notify(ticker, price);
    }

   private:
    void notify(const std::string& ticker, double price) {
        for (auto* obs : observers) obs->onPriceChange(ticker, price);
    }
};

/// Concrete Observers
struct InvestorDisplay : StockObserver {
    std::string name;
    explicit InvestorDisplay(std::string nam) : name(std::move(nam)) {}
    void onPriceChange(const std::string& ticker, double price) override {
        std::println("[{}] Updated dashboard: {} = ${}", name, ticker, price);
    }
};
struct AlertBot : StockObserver {
    double threshold;
    explicit AlertBot(double thr) : threshold(thr) {}
    void onPriceChange(const std::string& ticker, double price) override {
        if (price > threshold)
            std::println("[AlertBot] 🚨 {} exceeded ${}! Now at ${}", ticker, threshold, price);
    }
};

void demo() {
    std::println("\n Observer: Stock Price Tracker ");
    StockExchange   exchange;
    InvestorDisplay alice("Alice"), bob("Bob");
    AlertBot        bot(150.0);

    exchange.attach(&alice);
    exchange.attach(&bob);
    exchange.attach(&bot);

    exchange.setPrice("AAPL", 145.00);
    exchange.setPrice("AAPL", 160.00);

    std::println("[Bob unsubscribes]");
    exchange.detach(&bob);
    exchange.setPrice("AAPL", 170.00);
}

}  // namespace observer

/// ─────────────────────────────────────────────────────────────────────────────
/// PATTERN 20: STATE
/// ─────────────────────────────────────────────────────────────────────────────
///
/// PROJECT: Traffic Light State Machine
///   A traffic light cycles Red → Green → Yellow → Red. Each light's behavior
///   on tick() changes completely depending on the current state. Without State,
///   this becomes a large switch/if-else on a state enum scattered everywhere.
///   Adding a BlinkingRed (emergency) state means touching every switch.
///
/// WHY STATE?
///   ✓ The TrafficLight's behavior depends entirely on current state.
///   ✓ There are multiple state-dependent operations (tick, display).
///   ✓ Transitions are clean, well-defined, and need to be extended.
///
/// STATE vs STRATEGY:
///   State: transitions itself based on internal logic (auto-advance).
///   Strategy: set explicitly by the client; stays fixed until client changes it.
/// ─────────────────────────────────────────────────────────────────────────────
namespace state {

struct TrafficLight;  // forward

/// State interface
struct LightState {
    virtual void display(const TrafficLight&) const = 0;
    virtual void tick(TrafficLight&)                = 0;
    virtual ~LightState()                           = default;
};

/// Context
struct TrafficLight {
    std::unique_ptr<LightState> state;
    TrafficLight();  // defined after states
    void tick() { state->tick(*this); }
    void display() { state->display(*this); }
    void changeState(std::unique_ptr<LightState> newState) { state = std::move(newState); }
};

struct RedState : LightState {
    void display(const TrafficLight&) const override { std::println("🔴 RED - STOP"); }
    void tick(TrafficLight& light) override;
};
struct GreenState : LightState {
    void display(const TrafficLight&) const override { std::println("🟢 GREEN - GO"); }
    void tick(TrafficLight& light) override;
};
struct YellowState : LightState {
    void display(const TrafficLight&) const override { std::println("🟡 YELLOW - SLOW"); }
    void tick(TrafficLight& light) override;
};

// Transitions defined after all states are declared
void RedState::tick(TrafficLight& light) { light.changeState(std::make_unique<GreenState>()); }
void GreenState::tick(TrafficLight& light) { light.changeState(std::make_unique<YellowState>()); }
void YellowState::tick(TrafficLight& light) { light.changeState(std::make_unique<RedState>()); }

TrafficLight::TrafficLight() : state(std::make_unique<RedState>()) {}

void demo() {
    std::println("\n State: Traffic Light ");
    TrafficLight light;
    for (int idx = 0; idx < 6; ++idx) {
        std::println("Tick {}:", idx + 1);
        light.display();
        light.tick();
    }
}

}  // namespace state

/// ─────────────────────────────────────────────────────────────────────────────
/// PATTERN 21: STRATEGY
/// ─────────────────────────────────────────────────────────────────────────────
///
/// PROJECT: Sorting Algorithm Selector
///   A data processing tool needs to sort arrays. Different datasets perform
///   differently with different algorithms (Bubble for tiny sets, Quick for
///   medium, Merge for large/stable). The client selects the algorithm at
///   runtime based on data size; the sorting logic stays separate from the
///   data processing pipeline.
///
/// WHY STRATEGY?
///   ✓ Family of related algorithms (sort methods) that must be interchangeable.
///   ✓ The algorithm should be selectable at runtime, not hard-coded.
///   ✓ Eliminates a large conditional: if (size < 10) bubble(); else quick();
///
/// STRATEGY vs TEMPLATE METHOD:
///   Template Method: uses inheritance to vary STEPS of an algorithm
///   (compile-time). Strategy: uses composition to replace the WHOLE algorithm
///   (runtime). Choose Strategy when the algorithm itself must vary at runtime.
/// ─────────────────────────────────────────────────────────────────────────────
namespace strategy {

using Vec = std::vector<int>;

/// Strategy interface
struct SortStrategy {
    virtual void        sort(Vec& data) const = 0;
    virtual std::string name() const          = 0;
    virtual ~SortStrategy()                   = default;
};

struct BubbleSort : SortStrategy {
    void sort(Vec& dat) const override {
        for (size_t idx = 0; idx < dat.size(); ++idx)
            for (size_t jdx = 0; jdx + 1 < dat.size() - idx; ++jdx)
                if (dat[jdx] > dat[jdx + 1])
                    std::swap(dat[jdx], dat[jdx + 1]);
    }
    std::string name() const override { return "BubbleSort"; }
};

struct QuickSort : SortStrategy {
    void sort(Vec& dat) const override { qsort(dat, 0, (int)dat.size() - 1); }
    void qsort(Vec& dat, int lft, int rgt) const {
        if (lft >= rgt)
            return;
        int pivot = dat[rgt], idx = lft - 1;
        for (int jdx = lft; jdx < rgt; ++jdx)
            if (dat[jdx] <= pivot)
                std::swap(dat[++idx], dat[jdx]);
        std::swap(dat[++idx], dat[rgt]);
        qsort(dat, lft, idx - 1);
        qsort(dat, idx + 1, rgt);
    }
    std::string name() const override { return "QuickSort"; }
};

struct MergeSort : SortStrategy {
    void sort(Vec& dat) const override { mergeSort(dat, 0, (int)dat.size() - 1); }
    void mergeSort(Vec& dat, int lft, int rgt) const {
        if (lft >= rgt)
            return;
        int mid = (lft + rgt) / 2;
        mergeSort(dat, lft, mid);
        mergeSort(dat, mid + 1, rgt);
        Vec tmp;
        int idx = lft, jdx = mid + 1;
        while (idx <= mid && jdx <= rgt)
            tmp.push_back(dat[idx] <= dat[jdx] ? dat[idx++] : dat[jdx++]);
        while (idx <= mid) tmp.push_back(dat[idx++]);
        while (jdx <= rgt) tmp.push_back(dat[jdx++]);
        for (int kdx = lft; kdx <= rgt; ++kdx) dat[kdx] = tmp[kdx - lft];
    }
    std::string name() const override { return "MergeSort"; }
};

/// Context: holds a Strategy reference
struct DataSorter {
    std::unique_ptr<SortStrategy> strategy;

    void setStrategy(std::unique_ptr<SortStrategy> strat) { strategy = std::move(strat); }

    void sort(Vec& data) {
        std::println("Sorting {} items with {}:", data.size(), strategy->name());
        strategy->sort(data);
        std::print("  Result:");
        for (int val : data) std::print(" {}", val);
        std::println("");
    }
};

/// Smart selector: pick algorithm based on data characteristics
std::unique_ptr<SortStrategy> selectStrategy(size_t num) {
    if (num <= 8)
        return std::make_unique<BubbleSort>();
    if (num <= 100)
        return std::make_unique<QuickSort>();
    return std::make_unique<MergeSort>();
}

void demo() {
    std::println("\n Strategy: Sorting Algorithm Selector ");
    DataSorter sorter;

    Vec tiny = {5, 2, 8, 1};
    sorter.setStrategy(selectStrategy(tiny.size()));
    sorter.sort(tiny);

    Vec medium = {64, 25, 12, 22, 11, 90, 3, 47, 80, 55};
    sorter.setStrategy(selectStrategy(medium.size()));
    sorter.sort(medium);
}

}  // namespace strategy

/// ─────────────────────────────────────────────────────────────────────────────
/// PATTERN 22: TEMPLATE METHOD
/// ─────────────────────────────────────────────────────────────────────────────
///
/// PROJECT: Data Report Generator
///   A reporting system generates reports in different formats (CSV, HTML).
///   The steps are always: (1) fetchData, (2) parseData, (3) formatOutput,
///   (4) saveFile. Steps 1 and 4 are identical for all formats. Steps 2 and 3
///   vary. Template Method defines the skeleton in the base class.
///
/// WHY TEMPLATE METHOD?
///   ✓ The algorithm structure (steps 1-4) is invariant.
///   ✓ Only specific steps (parse, format) vary by subclass.
///   ✓ Avoids duplicating steps 1 and 4 across every report type.
///   ✓ The "Hollywood Principle" - the base class calls the subclass, not vice
///   versa.
///
/// TEMPLATE METHOD vs STRATEGY:
///   Template Method → inheritance, compile-time. The STRUCTURE is fixed;
///                     subclasses customize STEPS. Less overhead.
///   Strategy        → composition, runtime. The WHOLE algorithm can be swapped.
///   Choose Template Method when you control the class hierarchy and structure
///   is truly invariant. Choose Strategy when the algorithm itself varies at
///   runtime.
/// ─────────────────────────────────────────────────────────────────────────────
namespace template_method {

struct ReportGenerator {
    /// Template Method: the invariant skeleton (final to prevent overriding)
    void generate(const std::string& title) {
        auto raw    = fetchData(title);           // same for all
        auto data   = parseData(raw);             // varies
        auto output = formatOutput(title, data);  // varies
        saveFile(output);                         // same for all
    }

    virtual ~ReportGenerator() = default;

   protected:
    /// Invariant steps (concrete in base)
    std::string fetchData(const std::string& title) {
        std::println("[Base] Fetching data for: {}", title);
        return "row1|col1|col2\nrow2|col3|col4";
    }
    void saveFile(const std::string& output) {
        std::println("[Base] Saving output ({} chars)", output.size());
    }

    /// Variant steps (abstract - must be implemented by subclasses)
    virtual std::vector<std::string> parseData(const std::string& raw) = 0;
    virtual std::string              formatOutput(
        const std::string& title, const std::vector<std::string>& rows
    ) = 0;
};

struct CSVReport : ReportGenerator {
   protected:
    std::vector<std::string> parseData(const std::string& raw) override {
        std::println("[CSV] Parsing raw data");
        std::vector<std::string> rows;
        std::istringstream       iss(raw);
        std::string              line;
        while (std::getline(iss, line)) rows.push_back(line);
        return rows;
    }
    std::string formatOutput(
        const std::string& title, const std::vector<std::string>& rows
    ) override {
        std::println("[CSV] Formatting as CSV");
        std::string out = "# " + title + "\n";
        for (const auto& rawRow : rows) {
            auto row = rawRow;
            std::replace(row.begin(), row.end(), '|', ',');
            out += row + "\n";
        }
        return out;
    }
};

struct HTMLReport : ReportGenerator {
   protected:
    std::vector<std::string> parseData(const std::string& raw) override {
        std::println("[HTML] Parsing raw data");
        std::vector<std::string> rows;
        std::istringstream       iss(raw);
        std::string              line;
        while (std::getline(iss, line)) rows.push_back(line);
        return rows;
    }
    std::string formatOutput(
        const std::string& title, const std::vector<std::string>& rows
    ) override {
        std::println("[HTML] Formatting as HTML");
        std::string out = "<h1>" + title + "</h1><table>";
        for (const auto& row : rows) out += "<tr><td>" + row + "</td></tr>";
        out += "</table>";
        return out;
    }
};

void demo() {
    std::println("\n Template Method: Report Generator ");
    CSVReport  csv;
    HTMLReport html;

    std::println("Generating CSV report:");
    csv.generate("Q4 Sales");
    std::println("Generating HTML report:");
    html.generate("Q4 Sales");
}

}  // namespace template_method

/// ─────────────────────────────────────────────────────────────────────────────
/// PATTERN 23: VISITOR
/// ─────────────────────────────────────────────────────────────────────────────
///
/// PROJECT: Shopping Cart - Tax & Discount Calculator
///   A shopping cart contains Electronics and Food items. We need multiple
///   operations: (1) calculate total price, (2) calculate tax, (3) apply
///   discounts. Adding these methods to every item class would pollute them.
///   The set of item types is stable; the number of operations grows over time.
///
/// WHY VISITOR?
///   ✓ Many distinct operations (price, tax, discount) over a stable set of
///     element types (Electronics, Food). Classic Visitor use case.
///   ✓ Adding a new operation = add one Visitor class; no item classes change.
///   ✓ Related logic is co-located in one Visitor (all tax logic in TaxVisitor).
///
/// CRITICAL TRADE-OFF:
///   Visitor is the INVERSE of Interpreter's trade-off:
///     Adding new OPERATIONS: easy with Visitor, hard with Interpreter.
///     Adding new ELEMENT TYPES: easy with Interpreter, hard with Visitor
///                               (every ConcreteVisitor must add a new Visit
///                               method).
///
/// DOUBLE DISPATCH:
///   element.accept(visitor) → visitor.visit(this)
///   Both the element type AND the visitor type determine which code runs.
/// ─────────────────────────────────────────────────────────────────────────────
namespace visitor {

struct Electronics;  // forward declarations for Visitor
struct Food;

/// Visitor interface: one Visit method per element type
struct CartVisitor {
    virtual void visit(const Electronics& ele) = 0;
    virtual void visit(const Food& fod)        = 0;
    virtual ~CartVisitor()                     = default;
};

/// Element interface
struct CartItem {
    std::string name;
    double      price;
    CartItem(std::string nam, double prc) : name(std::move(nam)), price(prc) {}
    virtual void accept(CartVisitor& vis) const = 0;
    virtual ~CartItem()                         = default;
};

/// Concrete Elements
struct Electronics : CartItem {
    using CartItem::CartItem;
    void accept(CartVisitor& vis) const override { vis.visit(*this); }
};
struct Food : CartItem {
    using CartItem::CartItem;
    void accept(CartVisitor& vis) const override { vis.visit(*this); }
};

/// Concrete Visitor 1: Calculate total price
struct PriceVisitor : CartVisitor {
    double total = 0;
    void   visit(const Electronics& ele) override {
        total += ele.price;
        std::println("Electronics: {} ${}", ele.name, ele.price);
    }
    void visit(const Food& fod) override {
        total += fod.price;
        std::println("Food:        {} ${}", fod.name, fod.price);
    }
};

/// Concrete Visitor 2: Calculate tax (Electronics 15%, Food 5%)
struct TaxVisitor : CartVisitor {
    double totalTax = 0;
    void   visit(const Electronics& ele) override {
        double tax = ele.price * 0.15;
        totalTax += tax;
        std::println("Electronics tax ({}): ${} (15%)", ele.name, tax);
    }
    void visit(const Food& fod) override {
        double tax = fod.price * 0.05;
        totalTax += tax;
        std::println("Food tax ({}): ${} (5%)", fod.name, tax);
    }
};

/// Concrete Visitor 3: Apply discounts (Electronics 10% off, Food no discount)
struct DiscountVisitor : CartVisitor {
    double totalSaved = 0;
    void   visit(const Electronics& ele) override {
        double saved = ele.price * 0.10;
        totalSaved += saved;
        std::println("Electronics discount ({}): -${} (10% off)", ele.name, saved);
    }
    void visit(const Food& fod) override { std::println("No discount on {}", fod.name); }
};

void demo() {
    std::println("\n Visitor: Shopping Cart ");
    std::vector<std::unique_ptr<CartItem>> cart;
    cart.push_back(std::make_unique<Electronics>("Laptop", 999.99));
    cart.push_back(std::make_unique<Food>("Organic Tea", 12.50));
    cart.push_back(std::make_unique<Electronics>("Headphones", 149.99));
    cart.push_back(std::make_unique<Food>("Coffee Beans", 24.00));

    std::println("\n-- Price Breakdown --");
    PriceVisitor prv;
    for (const auto& item : cart) item->accept(prv);
    std::println("Total: ${}", prv.total);

    std::println("\n-- Tax Calculation --");
    TaxVisitor taxV;
    for (const auto& item : cart) item->accept(taxV);
    std::println("Total Tax: ${}", taxV.totalTax);

    std::println("\n-- Discounts Applied --");
    DiscountVisitor dscV;
    for (const auto& item : cart) item->accept(dscV);
    std::println("Total Saved: ${}", dscV.totalSaved);
}

}  // namespace visitor

/// Main function
int main() {
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
#endif
    chain_of_responsibility::demo();
    command::demo();
    interpreter::demo();
    iterator::demo();
    mediator::demo();
    memento::demo();
    observer::demo();
    state::demo();
    strategy::demo();
    template_method::demo();
    visitor::demo();
    return 0;
}
