// =============================================================================
// STRUCTURAL PATTERNS - C++ Implementations
// Based on: Design Patterns: Elements of Reusable Object-Oriented Software
//           Erich Gamma, Richard Helm, Ralph Johnson, John Vlissides (GoF)
//
// Patterns covered (7):
//   Adapter, Bridge, Composite, Decorator, Facade, Flyweight, Proxy
// =============================================================================

#include <iostream>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

/// ─────────────────────────────────────────────────────────────────────────────
/// PATTERN 6: ADAPTER (Object form)
/// ─────────────────────────────────────────────────────────────────────────────
///
/// PROJECT: Legacy Payment Gateway Integration
///   Your app uses a PaymentProcessor interface (charge / refund). You need to
///   integrate StripeAPI, a third-party SDK with completely different method
///   names (makePayment / reverseCharge). You cannot modify the SDK.
///
/// WHY ADAPTER?
///   ✓ You have an existing class (StripeAPI / Adaptee) whose interface
///     doesn't match what the client expects (PaymentProcessor / Target).
///   ✓ You cannot change StripeAPI (third-party SDK).
///   ✓ The mismatch is interface-level, not a fundamental incompatibility.
///
/// OBJECT vs CLASS ADAPTER: Using the Object Adapter (composition) because
///   C++ multiple inheritance would work for Class Adapter but is less flexible;
///   this Adapter could wrap StripeSubclass without recompiling.
///
/// ALTERNATIVES RULED OUT:
///   Façade: Façade wraps a *subsystem*; here we adapt ONE object's interface.
///   Bridge: plans for multiple implementations upfront; this is
///           after-the-fact integration of an unforeseen third-party.
/// ─────────────────────────────────────────────────────────────────────────────
namespace adapter {

/// Target interface: what our application code expects
struct PaymentProcessor {
    virtual void charge(double amount) = 0;
    virtual void refund(double amount) = 0;
    virtual ~PaymentProcessor()        = default;
};

/// Adaptee: the existing third-party Stripe SDK (can't modify)
struct StripeAPI {
    void makePayment(double usd) { std::cout << "[Stripe] Processing payment of $" << usd << "\n"; }
    void reverseCharge(double usd) { std::cout << "[Stripe] Reversing charge of $" << usd << "\n"; }
};

/// Object Adapter: wraps StripeAPI, exposes PaymentProcessor interface
struct StripeAdapter : PaymentProcessor {
    explicit StripeAdapter(std::shared_ptr<StripeAPI> stripe) : stripe_(stripe) {}

    void charge(double amount) override { stripe_->makePayment(amount); }
    void refund(double amount) override { stripe_->reverseCharge(amount); }

   private:
    std::shared_ptr<StripeAPI> stripe_;
};

/// Client: only knows PaymentProcessor; unaware of Stripe internals
void processOrder(PaymentProcessor& p, double total) {
    std::cout << "Charging customer: ";
    p.charge(total);
}

void demo() {
    std::cout << "\n=== Adapter: Legacy Payment Gateway ===\n";
    auto          stripeSDK = std::make_shared<StripeAPI>();
    StripeAdapter adapter(stripeSDK);
    processOrder(adapter, 99.99);
    adapter.refund(15.00);
}

}  // namespace adapter

/// ─────────────────────────────────────────────────────────────────────────────
/// PATTERN 7: BRIDGE
/// ─────────────────────────────────────────────────────────────────────────────
///
/// PROJECT: Remote Control for Electronic Devices
///   You need remotes (BasicRemote, AdvancedRemote) that control devices (TV,
///   Radio, Projector). If Remote subclasses TV and Radio, you get N×M classes
///   (BasicTVRemote, AdvancedTVRemote, BasicRadioRemote…). Bridge flattens
///   this to N+M.
///
/// WHY BRIDGE?
///   ✓ Two independent dimensions that both need subclassing: Remote type and
///     Device type. Bridge was designed for exactly this N×M to N+M reduction.
///   ✓ You want to select the device at runtime (plug the remote into any
///   device). ✓ Both hierarchies will grow independently (new remotes, new
///   devices).
///
/// BRIDGE vs ADAPTER:
///   Adapter: retrofits after classes unexpectedly need to interoperate.
///   Bridge: planned upfront because you KNOW both axes will vary.
///   Slogan: "Adapter after; Bridge before."
/// ─────────────────────────────────────────────────────────────────────────────
namespace bridge {

/// Implementor interface: the "device" axis
struct Device {
    virtual void        powerOn()        = 0;
    virtual void        powerOff()       = 0;
    virtual void        setVolume(int v) = 0;
    virtual std::string name() const     = 0;
    virtual ~Device()                    = default;
};

struct TV : Device {
    void        powerOn() override { std::cout << "[TV] Power ON\n"; }
    void        powerOff() override { std::cout << "[TV] Power OFF\n"; }
    void        setVolume(int v) override { std::cout << "[TV] Volume → " << v << "\n"; }
    std::string name() const override { return "TV"; }
};
struct Radio : Device {
    void        powerOn() override { std::cout << "[Radio] Power ON\n"; }
    void        powerOff() override { std::cout << "[Radio] Power OFF\n"; }
    void        setVolume(int v) override { std::cout << "[Radio] Volume → " << v << "\n"; }
    std::string name() const override { return "Radio"; }
};

/// Abstraction: the "remote" axis; holds the bridge to a Device
struct Remote {
    explicit Remote(std::shared_ptr<Device> device) : device_(device) {}
    virtual void togglePower() { device_->powerOn(); }
    virtual void volumeUp() { device_->setVolume(volume_ += 10); }
    virtual ~Remote() = default;

   protected:
    std::shared_ptr<Device> device_;
    int                     volume_ = 50;
};

/// Refined Abstraction: extends Remote without touching Device
struct AdvancedRemote : Remote {
    explicit AdvancedRemote(std::shared_ptr<Device> device) : Remote(device) {}
    void mute() {
        std::cout << "[AdvancedRemote] Muting " << device_->name() << "\n";
        device_->setVolume(0);
    }
};

void demo() {
    std::cout << "\n=== Bridge: Remote Control & Devices ===\n";
    auto tv    = std::make_shared<TV>();
    auto radio = std::make_shared<Radio>();

    Remote basicTVRemote(tv);
    basicTVRemote.togglePower();
    basicTVRemote.volumeUp();

    AdvancedRemote advRadioRemote(radio);
    advRadioRemote.togglePower();
    advRadioRemote.mute();
}

}  // namespace bridge

/// ─────────────────────────────────────────────────────────────────────────────
/// PATTERN 8: COMPOSITE
/// ─────────────────────────────────────────────────────────────────────────────
///
/// PROJECT: File System (Files and Folders)
///   A file system has Files (leaves) and Folders (composites containing other
///   Files or Folders). Printing the size of a Folder should recursively sum
///   sizes of all children - identical call for both File and Folder.
///
/// WHY COMPOSITE?
///   ✓ We have a classic part-whole hierarchy (Folder contains Files and
///     Folders), and we want to treat them uniformly via one interface.
///   ✓ "Display the size of X" should work whether X is a file or a folder.
///   ✓ Client code shouldn't ask "is this a file or a folder?" repeatedly.
///
/// DESIGN DECISION: Child management (add/remove) placed on the Component
///   interface for maximum transparency - Files implement them as no-ops.
///   Alternative: put only on Composite for type-safety, but then clients must
///   downcast. The book leans toward transparency here.
/// ─────────────────────────────────────────────────────────────────────────────
namespace composite {

/// Component: uniform interface for both leaves and composites
struct FileSystemNode {
    std::string name;
    explicit FileSystemNode(std::string n) : name(std::move(n)) {}

    virtual long size() const                = 0;
    virtual void print(int indent = 0) const = 0;
    virtual void add(std::shared_ptr<FileSystemNode>) { /* no-op for leaves */ }
    virtual ~FileSystemNode() = default;
};

// Leaf
struct File : FileSystemNode {
    long bytes;
    File(std::string n, long b) : FileSystemNode(std::move(n)), bytes(b) {}
    long size() const override { return bytes; }
    void print(int indent) const override {
        std::cout << std::string(indent, ' ') << "📄 " << name << " (" << bytes << " B)\n";
    }
};

// Composite
struct Folder : FileSystemNode {
    std::vector<std::shared_ptr<FileSystemNode>> children;
    explicit Folder(std::string n) : FileSystemNode(std::move(n)) {}
    void add(std::shared_ptr<FileSystemNode> node) override { children.push_back(node); }
    long size() const override {
        long total = 0;
        for (const auto& c : children) total += c->size();
        return total;
    }
    void print(int indent) const override {
        std::cout << std::string(indent, ' ') << "📁 " << name << "/\n";
        for (const auto& c : children) c->print(indent + 4);
    }
};

void demo() {
    std::cout << "\n=== Composite: File System ===\n";

    auto root = std::make_shared<Folder>("root");
    auto src  = std::make_shared<Folder>("src");
    auto docs = std::make_shared<Folder>("docs");

    src->add(std::make_shared<File>("main.cpp", 4200));
    src->add(std::make_shared<File>("utils.cpp", 1800));
    docs->add(std::make_shared<File>("README.md", 900));
    root->add(src);
    root->add(docs);
    root->add(std::make_shared<File>(".gitignore", 120));

    root->print(0);
    std::cout << "Total size: " << root->size() << " B\n";
}

}  // namespace composite

/// ─────────────────────────────────────────────────────────────────────────────
/// PATTERN 9: DECORATOR
/// ─────────────────────────────────────────────────────────────────────────────
///
/// PROJECT: Coffee Shop Order System
///   Base beverages (Espresso, Drip Coffee) can have optional add-ons (Milk,
///   Vanilla Syrup, Whipped Cream). Every combination must compute cost and
///   description correctly. Subclassing each combination (EspressoWithMilk,
///   EspressoWithMilkAndVanilla…) would explode combinatorially.
///
/// WHY DECORATOR?
///   ✓ Responsibilities (add-ons) must be added dynamically and transparently.
///   ✓ Add-ons are optional and combinable - this is the textbook case.
///   ✓ Each Decorator wraps a Beverage and IS-A Beverage (same interface),
///     so they nest freely in any order.
///
/// DECORATOR vs INHERITANCE:
///   Inheritance gives you EspressoWithMilk at compile time.
///   Decorator gives you espresso + milk + vanilla at RUNTIME, mixed freely.
///
/// KEY LIABILITY: Decorator identity differs from the wrapped object.
///   `decorator == original` is false. Irrelevant here (we care about cost),
///   but to be aware of in identity-sensitive contexts.
/// ─────────────────────────────────────────────────────────────────────────────
namespace decorator {

// Component interface
struct Beverage {
    virtual double      cost() const        = 0;
    virtual std::string description() const = 0;
    virtual ~Beverage()                     = default;
};

// Concrete Components (base beverages)
struct Espresso : Beverage {
    double      cost() const override { return 2.50; }
    std::string description() const override { return "Espresso"; }
};
struct DripCoffee : Beverage {
    double      cost() const override { return 1.00; }
    std::string description() const override { return "Drip Coffee"; }
};

/// Base Decorator: wraps a Beverage, IS-A Beverage
struct AddOnDecorator : Beverage {
    explicit AddOnDecorator(std::unique_ptr<Beverage> b) : beverage_(std::move(b)) {}

   protected:
    std::unique_ptr<Beverage> beverage_;
};

// Concrete Decorators
struct Milk : AddOnDecorator {
    explicit Milk(std::unique_ptr<Beverage> b) : AddOnDecorator(std::move(b)) {}
    double      cost() const override { return beverage_->cost() + 0.30; }
    std::string description() const override { return beverage_->description() + ", Milk"; }
};
struct VanillaSyrup : AddOnDecorator {
    explicit VanillaSyrup(std::unique_ptr<Beverage> b) : AddOnDecorator(std::move(b)) {}
    double      cost() const override { return beverage_->cost() + 0.50; }
    std::string description() const override { return beverage_->description() + ", Vanilla"; }
};
struct WhippedCream : AddOnDecorator {
    explicit WhippedCream(std::unique_ptr<Beverage> b) : AddOnDecorator(std::move(b)) {}
    double      cost() const override { return beverage_->cost() + 0.75; }
    std::string description() const override {
        return beverage_->description() + ", Whipped Cream";
    }
};

void printOrder(const Beverage& b) { std::cout << b.description() << " - $" << b.cost() << "\n"; }

void demo() {
    std::cout << "\n=== Decorator: Coffee Shop Order System ===\n";

    // Plain espresso
    auto order1 = std::make_unique<Espresso>();
    printOrder(*order1);

    // Espresso + Milk + Vanilla
    std::unique_ptr<Beverage> order2 = std::make_unique<Espresso>();
    order2                           = std::make_unique<Milk>(std::move(order2));
    order2                           = std::make_unique<VanillaSyrup>(std::move(order2));
    printOrder(*order2);

    // Drip Coffee + Milk + Whipped Cream
    std::unique_ptr<Beverage> order3 = std::make_unique<DripCoffee>();
    order3                           = std::make_unique<Milk>(std::move(order3));
    order3                           = std::make_unique<WhippedCream>(std::move(order3));
    printOrder(*order3);
}

}  // namespace decorator

/// ─────────────────────────────────────────────────────────────────────────────
/// PATTERN 10: FAÇADE
/// ─────────────────────────────────────────────────────────────────────────────
///
/// PROJECT: Home Theater System
///   A home theater has Amplifier, DVDPlayer, Projector, Screen, Lights - each
///   with its own complex API. To watch a movie, a user must call 8+ methods in
///   the right order. The Façade wraps this into watchMovie() and endMovie().
///
/// WHY FAÇADE?
///   ✓ Many subsystem classes with complex coordination for common tasks.
///   ✓ Clients want one simple interface without learning all subsystem classes.
///   ✓ Power users can still access subsystem classes directly.
///
/// FAÇADE vs MEDIATOR:
///   Façade: subsystem classes DON'T know about the Façade. One-way.
///   Mediator: colleagues actively communicate THROUGH the Mediator. Two-way.
///
/// FAÇADE vs ADAPTER:
///   Adapter: translates one object's interface. Façade defines a NEW
///            simplified interface over a whole subsystem.
/// ─────────────────────────────────────────────────────────────────────────────
namespace facade {

// Subsystem classes - complex, fine-grained
struct Amplifier {
    void on() { std::cout << "  Amplifier ON\n"; }
    void setVolume(int v) { std::cout << "  Amplifier volume → " << v << "\n"; }
    void off() { std::cout << "  Amplifier OFF\n"; }
};
struct DVDPlayer {
    void on() { std::cout << "  DVD Player ON\n"; }
    void play(const std::string& m) { std::cout << "  DVD playing: " << m << "\n"; }
    void stop() { std::cout << "  DVD stopped\n"; }
    void off() { std::cout << "  DVD Player OFF\n"; }
};
struct Projector {
    void on() { std::cout << "  Projector ON\n"; }
    void widescreen() { std::cout << "  Projector → widescreen mode\n"; }
    void off() { std::cout << "  Projector OFF\n"; }
};
struct Screen {
    void lower() { std::cout << "  Screen lowering...\n"; }
    void raise() { std::cout << "  Screen raising...\n"; }
};
struct Lights {
    void dim(int level) { std::cout << "  Lights dimmed to " << level << "%\n"; }
    void on() { std::cout << "  Lights ON\n"; }
};

/// Façade: simple interface over the whole subsystem
struct HomeTheaterFacade {
    Amplifier amp;
    DVDPlayer dvd;
    Projector proj;
    Screen    screen;
    Lights    lights;

    void watchMovie(const std::string& movie) {
        std::cout << "Getting ready to watch \"" << movie << "\"...\n";
        lights.dim(10);
        screen.lower();
        proj.on();
        proj.widescreen();
        amp.on();
        amp.setVolume(30);
        dvd.on();
        dvd.play(movie);
    }

    void endMovie() {
        std::cout << "Shutting down theater...\n";
        dvd.stop();
        dvd.off();
        amp.off();
        proj.off();
        screen.raise();
        lights.on();
    }
};

void demo() {
    std::cout << "\n=== Façade: Home Theater System ===\n";
    HomeTheaterFacade theater;
    theater.watchMovie("Interstellar");
    std::cout << "... watching ...\n";
    theater.endMovie();
}

}  // namespace facade

/// ─────────────────────────────────────────────────────────────────────────────
/// PATTERN 11: FLYWEIGHT
/// ─────────────────────────────────────────────────────────────────────────────
///
/// PROJECT: Forest Renderer (Particle/Nature System)
///   Rendering a forest of 100,000 trees. Each tree has a species (name,
///   texture, color) - the INTRINSIC state - shared across all trees of that
///   species, plus a position (x, y) - the EXTRINSIC state - unique to each.
///   Without Flyweight: 100,000 objects each storing full texture data.
///   With Flyweight: 3-5 shared TreeType objects; positions passed in.
///
/// WHY FLYWEIGHT?
///   ✓ Large number of objects (100k+ trees).
///   ✓ Most state (species data, textures) is intrinsic (context-independent)
///     and shared across thousands of instances.
///   ✓ Identity does NOT matter - two Oaks at different positions are the same
///     TreeType flyweight.
///
/// INTRINSIC vs EXTRINSIC:
///   Intrinsic  → stored IN the flyweight (species, texture, color)
///   Extrinsic  → passed by the client when needed (x, y coordinates)
/// ─────────────────────────────────────────────────────────────────────────────
namespace flyweight {

/// Flyweight: stores INTRINSIC (shared) state only
struct TreeType {
    std::string species, texture, color;
    TreeType(std::string s, std::string t, std::string c)
        : species(std::move(s)), texture(std::move(t)), color(std::move(c)) {}

    // Extrinsic state (x, y) passed in - NOT stored here
    void draw(int x, int y) const {
        std::cout << "[" << species << "@(" << x << "," << y << ")] "
                  << "color=" << color << " tex=" << texture << "\n";
    }
};

/// FlyweightFactory: ensures sharing; manages the pool
struct TreeFactory {
    std::unordered_map<std::string, std::shared_ptr<TreeType>> pool;

    std::shared_ptr<TreeType> getTreeType(
        const std::string& species, const std::string& texture, const std::string& color
    ) {
        std::string key = species + "|" + texture + "|" + color;
        auto        it  = pool.find(key);
        if (it == pool.end()) {
            std::cout << "  [Factory] Creating new TreeType: " << species << "\n";
            pool[key] = std::make_shared<TreeType>(species, texture, color);
        }
        return pool[key];
    }
    size_t typeCount() const { return pool.size(); }
};

/// Tree: a thin context object; holds position + reference to shared flyweight
struct Tree {
    int                       x, y;
    std::shared_ptr<TreeType> type;
    void                      draw() const { type->draw(x, y); }
};

void demo() {
    std::cout << "\n=== Flyweight: Forest Renderer ===\n";
    TreeFactory       factory;
    std::vector<Tree> forest;

    // Plant 6 trees of only 2 species - factory creates only 2 TreeType objects
    auto oak  = factory.getTreeType("Oak", "bark_oak.png", "dark_green");
    auto pine = factory.getTreeType("Pine", "bark_pine.png", "blue_green");
    factory.getTreeType("Oak", "bark_oak.png",
                        "dark_green");  // returned from pool

    for (auto [tx, ty] : std::vector<std::pair<int, int>> {{10, 20}, {30, 40}, {50, 10}})
        forest.push_back({tx, ty, oak});
    for (auto [tx, ty] : std::vector<std::pair<int, int>> {{15, 55}, {70, 30}, {5, 80}})
        forest.push_back({tx, ty, pine});

    std::cout << "Drawing " << forest.size() << " trees using " << factory.typeCount()
              << " TreeType flyweights:\n";
    for (const auto& t : forest) t.draw();
}

}  // namespace flyweight

/// ─────────────────────────────────────────────────────────────────────────────
/// PATTERN 12: PROXY
/// ─────────────────────────────────────────────────────────────────────────────
///
/// PROJECT: Virtual Image Loader (Lazy Loading Proxy)
///   A document editor displays images inline. Loading all images at startup
///   is slow - most won't be seen immediately. A Virtual Proxy stands in for
///   the real Image, providing size info for layout, but defers the expensive
///   disk load until the image is actually drawn on screen.
///
/// WHY PROXY (Virtual)?
///   ✓ The real object (Image) is expensive to create (disk I/O + decoding).
///   ✓ Creation should be deferred until the object is actually needed.
///   ✓ The Proxy exposes the SAME interface as the real Subject - the client
///     never knows whether it's talking to a real image or a proxy.
///
/// PROXY vs DECORATOR:
///   Both wrap an object with the same interface.
///   Decorator ADDS responsibilities.
///   Proxy CONTROLS ACCESS (lazy load, remote, protection, caching).
///   Intent is what differs.
/// ─────────────────────────────────────────────────────────────────────────────
namespace proxy {

// Subject interface
struct Image {
    virtual void draw() const   = 0;
    virtual int  width() const  = 0;
    virtual int  height() const = 0;
    virtual ~Image()            = default;
};

/// Real Subject: expensive to construct
struct RealImage : Image {
    std::string filename;
    int         w_, h_;
    explicit RealImage(std::string f) : filename(std::move(f)), w_(1920), h_(1080) {
        // Simulate slow disk load
        std::cout << "  [RealImage] Loading '" << filename << "' from disk...\n";
    }
    void draw() const override { std::cout << "  [RealImage] Drawing '" << filename << "'\n"; }
    int  width() const override { return w_; }
    int  height() const override { return h_; }
};

/// Virtual Proxy: same interface; defers RealImage creation
struct ImageProxy : Image {
    explicit ImageProxy(std::string f) : filename_(std::move(f)), real_(nullptr) {}

    void draw() const override {
        if (!real_) {
            std::cout << "  [Proxy] First draw - loading real image now\n";
            real_ = std::make_unique<RealImage>(filename_);
        }
        real_->draw();
    }
    int width() const override { return 1920; }  // known without loading
    int height() const override { return 1080; }

   private:
    std::string                        filename_;
    mutable std::unique_ptr<RealImage> real_;  // lazily initialized
};

void demo() {
    std::cout << "\n=== Proxy: Virtual Image Loader ===\n";
    // Three images created (no disk I/O yet)
    std::vector<std::unique_ptr<Image>> images;
    images.push_back(std::make_unique<ImageProxy>("photo1.jpg"));
    images.push_back(std::make_unique<ImageProxy>("photo2.jpg"));
    images.push_back(std::make_unique<ImageProxy>("photo3.jpg"));

    std::cout << "All image proxies created. No disk reads yet.\n";
    std::cout << "Layout uses width/height without loading: " << images[0]->width() << "x"
              << images[0]->height() << "\n";

    std::cout << "\nUser scrolls to see image 0 and image 2:\n";
    images[0]->draw();  // loads photo1.jpg
    images[2]->draw();  // loads photo3.jpg
    // photo2.jpg never loaded - user never scrolled there

    std::cout << "\nDrawing image 0 again (already loaded, no disk I/O):\n";
    images[0]->draw();
}

}  // namespace proxy

// ─────────────────────────────────────────────────────────────────────────────
int main() {
    adapter::demo();
    bridge::demo();
    composite::demo();
    decorator::demo();
    facade::demo();
    flyweight::demo();
    proxy::demo();
    return 0;
}
