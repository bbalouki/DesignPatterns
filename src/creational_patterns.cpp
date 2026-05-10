// =============================================================================
// CREATIONAL PATTERNS - C++ Implementations
// Based on: Design Patterns: Elements of Reusable Object-Oriented Software
//           Erich Gamma, Richard Helm, Ralph Johnson, John Vlissides (GoF)
//
// Each pattern section contains:
//   PROJECT: a real-world mini-project
//   WHY: decision process: why this pattern fits
//   DECISION: what alternatives were ruled out and why
//   CODE: full C++ implementation with demo
// =============================================================================

#include <cassert>
#include <iostream>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

/// ─────────────────────────────────────────────────────────────────────────────
/// PATTERN 1: ABSTRACT FACTORY
/// ─────────────────────────────────────────────────────────────────────────────
///
/// PROJECT: Cross-Platform UI Widget Toolkit
///   A desktop app needs to render Buttons and Checkboxes. It must look native
///   on both Windows and macOS without scattering "if (windows)…else…" checks
///   everywhere. The set of widget *kinds* (Button, Checkbox) is stable, but
///   more look-and-feel themes may be added later.
///
/// WHY ABSTRACT FACTORY?
///   ✓ We have a FAMILY of related products (Button + Checkbox) that must
///     match - we never want a macOS button next to a Windows checkbox.
///   ✓ We want to swap the whole theme by swapping one factory object.
///   ✓ Client code should never call `new WindowsButton` directly.
///
/// ALTERNATIVES RULED OUT:
///   Factory Method alone: only handles one product per creator; can't enforce
///                          family consistency across Button + Checkbox
///                          together.
///   Direct if/else: spreads platform knowledge everywhere; nightmare
///                          when a third theme (Linux GTK) is added.
///
/// KEY TRADE-OFF: Adding a *new kind* of widget (e.g., Slider) requires touching
/// every concrete factory - this is the pattern's known liability.
/// ─────────────────────────────────────────────────────────────────────────────
namespace abstract_factory {

/// Abstract products
struct Button {
    virtual void render() const = 0;
    virtual ~Button()           = default;
};
struct Checkbox {
    virtual void render() const = 0;
    virtual ~Checkbox()         = default;
};

/// Concrete products - Windows family
struct WindowsButton : Button {
    void render() const override { std::cout << "[Windows] Rendering a rectangular button\n"; }
};
struct WindowsCheckbox : Checkbox {
    void render() const override { std::cout << "[Windows] Rendering a square checkbox\n"; }
};

/// Concrete products - macOS family
struct MacButton : Button {
    void render() const override { std::cout << "[macOS] Rendering a rounded button\n"; }
};
struct MacCheckbox : Checkbox {
    void render() const override { std::cout << "[macOS] Rendering a circular checkbox\n"; }
};

/// Abstract factory: one creation method per product KIND
struct WidgetFactory {
    virtual std::unique_ptr<Button>   createButton() const   = 0;
    virtual std::unique_ptr<Checkbox> createCheckbox() const = 0;
    virtual ~WidgetFactory()                                 = default;
};

/// Concrete factories: one per FAMILY
struct WindowsFactory : WidgetFactory {
    std::unique_ptr<Button> createButton() const override {
        return std::make_unique<WindowsButton>();
    }
    std::unique_ptr<Checkbox> createCheckbox() const override {
        return std::make_unique<WindowsCheckbox>();
    }
};
struct MacFactory : WidgetFactory {
    std::unique_ptr<Button> createButton() const override { return std::make_unique<MacButton>(); }
    std::unique_ptr<Checkbox> createCheckbox() const override {
        return std::make_unique<MacCheckbox>();
    }
};

/// Client: works only through the abstract factory; never touches concrete types
void renderUI(const WidgetFactory& factory) {
    auto btn = factory.createButton();
    auto chk = factory.createCheckbox();
    btn->render();
    chk->render();
}

void demo() {
    std::cout << "\n=== Abstract Factory: Cross-Platform UI ===\n";
    WindowsFactory winFac;
    MacFactory     macFac;
    std::cout << "On Windows:\n";
    renderUI(winFac);
    std::cout << "On macOS:\n";
    renderUI(macFac);
}

}  // namespace abstract_factory

/// ─────────────────────────────────────────────────────────────────────────────
/// PATTERN 2: BUILDER
/// ─────────────────────────────────────────────────────────────────────────────
///
/// PROJECT: Custom Computer Configurator
///   Users assemble a PC by choosing CPU, RAM, storage, and GPU. Not all
///   fields are required (a server skips GPU; a workstation skips gaming GPU).
///   The final Product is a complex object assembled step-by-step.
///
/// WHY BUILDER?
///   ✓ The product (Computer) has 4+ independent optional parts.
///   ✓ The same Director algorithm produces a GamingPC and a ServerPC just by
///     swapping the ConcreteBuilder - construction process is shared.
///   ✓ A telescoping constructor (Computer(cpu, ram, storage, gpu, cooling…))
///     would be unreadable and fragile.
///
/// ALTERNATIVES RULED OUT:
///   Abstract Factory: creates *families* in one call; can't build one complex
///                       object part-by-part.
///   Factory Method: no incremental assembly; just returns one object.
///
/// KEY TRADE-OFF: More classes than a simple constructor, but the Director
/// separates "how to build" from "what is built," which is the payoff.
/// ─────────────────────────────────────────────────────────────────────────────
namespace builder {

struct Computer {
    std::string cpu, ram, storage, gpu;
    void        describe() const {
        std::cout << "Computer: CPU=" << cpu << " RAM=" << ram << " Storage=" << storage
                  << " GPU=" << (gpu.empty() ? "None" : gpu) << "\n";
    }
};

/// Builder interface: declares every possible construction step
struct ComputerBuilder {
    virtual void     setCPU(const std::string& cpu)   = 0;
    virtual void     setRAM(const std::string& ram)   = 0;
    virtual void     setStorage(const std::string& s) = 0;
    virtual void     setGPU(const std::string& gpu)   = 0;
    virtual Computer getResult()                      = 0;
    virtual ~ComputerBuilder()                        = default;
};

/// ConcreteBuilder: assembles parts into a Computer
struct PCBuilder : ComputerBuilder {
    Computer pc;
    void     setCPU(const std::string& cpu) override { pc.cpu = cpu; }
    void     setRAM(const std::string& ram) override { pc.ram = ram; }
    void     setStorage(const std::string& s) override { pc.storage = s; }
    void     setGPU(const std::string& gpu) override { pc.gpu = gpu; }
    Computer getResult() override { return pc; }
};

/// Director: knows the correct order/steps for each configuration
struct Director {
    ComputerBuilder* builder = nullptr;

    void buildGamingPC() {
        builder->setCPU("Intel i9-14900K");
        builder->setRAM("64GB DDR5");
        builder->setStorage("2TB NVMe SSD");
        builder->setGPU("RTX 4090");
    }
    void buildServer() {
        builder->setCPU("AMD EPYC 9654");
        builder->setRAM("512GB ECC DDR5");
        builder->setStorage("8TB NVMe RAID");
        // no GPU - servers rarely need one
    }
};

void demo() {
    std::cout << "\n=== Builder: Custom Computer Configurator ===\n";
    PCBuilder b;
    Director  d;
    d.builder = &b;

    d.buildGamingPC();
    b.getResult().describe();

    b = {};  // reset builder for new configuration
    d.buildServer();
    b.getResult().describe();
}

}  // namespace builder

/// ─────────────────────────────────────────────────────────────────────────────
/// PATTERN 3: FACTORY METHOD
/// ─────────────────────────────────────────────────────────────────────────────
///
/// PROJECT: Game Enemy Spawner
///   A game has different levels: ForestLevel spawns Goblins, DungeonLevel
///   spawns Skeletons. The level class orchestrates enemy waves (same logic),
///   but each subclass decides which Enemy to spawn. New levels added without
///   touching wave logic.
///
/// WHY FACTORY METHOD?
///   ✓ The Creator (Level) has an algorithm (spawnWave) that needs an Enemy,
///     but can't predict which concrete Enemy type it will use.
///   ✓ Subclasses (ForestLevel, DungeonLevel) should decide the product -
///     this is the textbook Factory Method scenario.
///   ✓ The wave-spawning logic is shared; only the enemy type varies.
///
/// ALTERNATIVES RULED OUT:
///   Abstract Factory: we have one product (Enemy), not a family; overkill.
///   Prototype: would work, but requires every level to hold a
///                       prototype object; Factory Method is simpler here.
///   Direct new: hard-codes the concrete type; levels can't be reused
///                       in a different enemy context.
/// ─────────────────────────────────────────────────────────────────────────────
namespace factory_method {

struct Enemy {
    virtual void attack() const = 0;
    virtual ~Enemy()            = default;
};

struct Goblin : Enemy {
    void attack() const override { std::cout << "Goblin attacks with a club!\n"; }
};
struct Skeleton : Enemy {
    void attack() const override { std::cout << "Skeleton attacks with a sword!\n"; }
};
struct Dragon : Enemy {
    void attack() const override { std::cout << "Dragon breathes fire!\n"; }
};

/// Creator: contains the template algorithm; declares the factory method
struct Level {
    virtual std::unique_ptr<Enemy> createEnemy() const = 0;  // <- factory method

    /// Template algorithm that uses the factory method
    void spawnWave(int count) const {
        std::cout << "Spawning wave of " << count << " enemies:\n";
        for (int i = 0; i < count; ++i) {
            auto e = createEnemy();
            e->attack();
        }
    }
    virtual ~Level() = default;
};

/// Concrete creators: override only the factory method
struct ForestLevel : Level {
    std::unique_ptr<Enemy> createEnemy() const override { return std::make_unique<Goblin>(); }
};
struct DungeonLevel : Level {
    std::unique_ptr<Enemy> createEnemy() const override { return std::make_unique<Skeleton>(); }
};
struct BossLevel : Level {
    std::unique_ptr<Enemy> createEnemy() const override { return std::make_unique<Dragon>(); }
};

void demo() {
    std::cout << "\n=== Factory Method: Game Enemy Spawner ===\n";
    ForestLevel  forest;
    DungeonLevel dungeon;
    BossLevel    boss;

    std::cout << "[Forest Level]\n";
    forest.spawnWave(2);
    std::cout << "[Dungeon Level]\n";
    dungeon.spawnWave(2);
    std::cout << "[Boss Level]\n";
    boss.spawnWave(1);
}

}  // namespace factory_method

/// ─────────────────────────────────────────────────────────────────────────────
/// PATTERN 4: PROTOTYPE
/// ─────────────────────────────────────────────────────────────────────────────
///
/// PROJECT: Magic Spell System (Spell Palette)
///   A game has a palette of preconfigured spells (Fireball lv3, Ice Storm
///   lv5, etc.). When a player casts a spell, the system clones the registered
///   prototype rather than recreating the spell from scratch. Runtime-loaded
///   spell mods can register new prototypes without new C++ subclasses.
///
/// WHY PROTOTYPE?
///   ✓ Spells vary in *state* (level, damage, duration), not just class.
///     Two Fireballs at different levels shouldn't require two subclasses.
///   ✓ Spell prototypes are registered at runtime (from config/mods).
///   ✓ Cloning is cheaper than reconfiguring from scratch for complex objects.
///
/// ALTERNATIVES RULED OUT:
///   Factory Method: would need one Level subclass per spell variant; class
///                     explosion (FireballLv1, FireballLv2, FireballLv3…).
///   Abstract Factory: for families, not state-varied singletons.
///   Direct new: requires knowing the concrete class at call site;
///                       mods can't register new types at runtime.
/// ─────────────────────────────────────────────────────────────────────────────
namespace prototype {

struct Spell {
    std::string name;
    int         damage, duration;

    virtual std::unique_ptr<Spell> clone() const = 0;
    virtual void                   describe() const {
        std::cout << "Spell: " << name << " | DMG=" << damage << " | DUR=" << duration << "s\n";
    }
    virtual ~Spell() = default;
};

struct Fireball : Spell {
    std::unique_ptr<Spell> clone() const override {
        return std::make_unique<Fireball>(*this);  // copy constructor
    }
};
struct IceStorm : Spell {
    std::unique_ptr<Spell> clone() const override { return std::make_unique<IceStorm>(*this); }
};

/// PrototypeManager (registry): maps string keys to prototype instances
struct SpellRegistry {
    std::unordered_map<std::string, std::unique_ptr<Spell>> registry;

    void registerSpell(const std::string& key, std::unique_ptr<Spell> spell) {
        registry[key] = std::move(spell);
    }
    std::unique_ptr<Spell> cast(const std::string& key) const {
        auto it = registry.find(key);
        if (it == registry.end()) {
            std::cerr << "Unknown spell: " << key << "\n";
            return nullptr;
        }
        return it->second->clone();  // clone the prototype
    }
};

void demo() {
    std::cout << "\n=== Prototype: Magic Spell Palette ===\n";
    SpellRegistry reg;

    // Register prototype spells
    auto fb      = std::make_unique<Fireball>();
    fb->name     = "Fireball";
    fb->damage   = 80;
    fb->duration = 3;
    auto is      = std::make_unique<IceStorm>();
    is->name     = "Ice Storm";
    is->damage   = 60;
    is->duration = 7;
    reg.registerSpell("fireball", std::move(fb));
    reg.registerSpell("icestorm", std::move(is));

    // Cast = clone prototype and use
    auto cast1 = reg.cast("fireball");
    cast1->describe();
    auto cast2 = reg.cast("icestorm");
    cast2->describe();

    // Modify the clone without affecting the prototype
    auto cast3    = reg.cast("fireball");
    cast3->damage = 120;  // power-up for this cast only
    cast3->name   = "MEGA Fireball";
    cast3->describe();
    reg.cast("fireball")->describe();  // original prototype unchanged
}

}  // namespace prototype

/// ─────────────────────────────────────────────────────────────────────────────
/// PATTERN 5: SINGLETON
/// ─────────────────────────────────────────────────────────────────────────────
///
/// PROJECT: Application Configuration Manager
///   A desktop app reads its settings (theme, language, max_threads) from a
///   config file once at startup. Many subsystems (UI, networking, logging)
///   need to read these settings. There must be exactly one config object -
///   two instances would risk reading different values or double-parsing the
///   file.
///
/// WHY SINGLETON?
///   ✓ Uniqueness is a *domain requirement*, not a convenience: two config
///     instances parsing the same file would be a bug.
///   ✓ Global accessibility is needed by disparate subsystems.
///   ✓ Lazy initialization defers disk I/O until first use.
///
/// ALTERNATIVES RULED OUT:
///   Global variable: doesn't enforce uniqueness; anyone can create another.
///   Dependency injection of the same object: perfectly valid and more
///   testable, but here uniqueness enforcement justifies the Singleton.
///
/// WARNING APPLIED: Singleton makes unit testing harder. Use it only when
/// uniqueness is a genuine system constraint, not just "convenient."
///
/// THREAD SAFETY: Using C++11 Meyers Singleton - local static initialization
/// is guaranteed thread-safe by the standard.
/// ─────────────────────────────────────────────────────────────────────────────
namespace singleton {

class AppConfig {
   public:
    /// Meyers Singleton - thread-safe in C++11 and later
    static AppConfig& instance() {
        static AppConfig inst;  // initialized exactly once, lazily
        return inst;
    }

    /// Delete copy/move to prevent accidental duplication
    AppConfig(const AppConfig&)            = delete;
    AppConfig& operator=(const AppConfig&) = delete;

    /// Configuration values (in a real app, loaded from a file)
    std::string theme      = "dark";
    std::string language   = "en_US";
    int         maxThreads = 8;

    void describe() const {
        std::cout << "Config: theme=" << theme << " lang=" << language << " threads=" << maxThreads
                  << "\n";
    }

   private:
    AppConfig() {
        // Would parse config.json here
        std::cout << "[AppConfig] Loaded from disk (happens exactly once)\n";
    }
};

void demo() {
    std::cout << "\n=== Singleton: Application Config Manager ===\n";

    // Multiple call sites - all get the same instance
    AppConfig& cfg1 = AppConfig::instance();
    AppConfig& cfg2 = AppConfig::instance();
    cfg1.describe();

    cfg2.maxThreads = 16;  // change via cfg2…
    cfg1.describe();       // …visible through cfg1 - same object

    assert(&cfg1 == &cfg2);
    std::cout << "cfg1 and cfg2 are the same object: " << std::boolalpha << (&cfg1 == &cfg2)
              << "\n";
}

}  // namespace singleton

// ─────────────────────────────────────────────────────────────────────────────
// -----------------------------------------------------------------------------
int main() {
    abstract_factory::demo();
    builder::demo();
    factory_method::demo();
    prototype::demo();
    singleton::demo();
    return 0;
}
