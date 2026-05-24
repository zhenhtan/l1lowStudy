#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

namespace simple_factory {

class HwConstantsApi
{
public:
    virtual ~HwConstantsApi() = default;
    virtual auto GetBoardName() const -> std::string = 0;
    virtual auto GetMaxCarriers() const -> int = 0;
};

class MadeHwConstantsApi : public HwConstantsApi
{
public:
    auto GetBoardName() const -> std::string override
    {
        return "MADE";
    }

    auto GetMaxCarriers() const -> int override
    {
        return 4;
    }
};

class MarsHwConstantsApi : public HwConstantsApi
{
public:
    auto GetBoardName() const -> std::string override
    {
        return "MARS";
    }

    auto GetMaxCarriers() const -> int override
    {
        return 8;
    }
};

enum class HardwareVariant
{
    Made,
    Mars
};

auto CreateHwConstantsApi(HardwareVariant variant) -> std::unique_ptr<HwConstantsApi>
{
    switch (variant)
    {
        case HardwareVariant::Made:
            return std::make_unique<MadeHwConstantsApi>();
        case HardwareVariant::Mars:
            return std::make_unique<MarsHwConstantsApi>();
        default:
            throw std::invalid_argument("Unsupported hardware variant");
    }
}

void Run()
{
    const auto api = CreateHwConstantsApi(HardwareVariant::Mars);
    std::cout << "[Simple Factory]\n";
    std::cout << "board=" << api->GetBoardName()
              << ", maxCarriers=" << api->GetMaxCarriers() << "\n\n";
}

} // namespace simple_factory

namespace abstract_factory {

struct StartupObject
{
    std::string name;
    std::string payload;
};

enum class TransactionType
{
    CreateCarrier,
    CreateBeam
};

class IStaticHoCreator
{
public:
    virtual ~IStaticHoCreator() = default;
    virtual auto MakeObjects() -> std::vector<std::unique_ptr<StartupObject>> = 0;
    virtual auto GetTransactionType() const -> TransactionType = 0;
};

class CarrierCreator : public IStaticHoCreator
{
public:
    auto MakeObjects() -> std::vector<std::unique_ptr<StartupObject>> override
    {
        std::vector<std::unique_ptr<StartupObject>> objects;
        objects.push_back(std::make_unique<StartupObject>(StartupObject{"carrier-0", "100MHz"}));
        objects.push_back(std::make_unique<StartupObject>(StartupObject{"carrier-1", "200MHz"}));
        return objects;
    }

    auto GetTransactionType() const -> TransactionType override
    {
        return TransactionType::CreateCarrier;
    }
};

class BeamCreator : public IStaticHoCreator
{
public:
    auto MakeObjects() -> std::vector<std::unique_ptr<StartupObject>> override
    {
        std::vector<std::unique_ptr<StartupObject>> objects;
        objects.push_back(std::make_unique<StartupObject>(StartupObject{"beam-0", "azimuth=15"}));
        objects.push_back(std::make_unique<StartupObject>(StartupObject{"beam-1", "azimuth=30"}));
        return objects;
    }

    auto GetTransactionType() const -> TransactionType override
    {
        return TransactionType::CreateBeam;
    }
};

auto CreateStartupCreators() -> std::vector<std::unique_ptr<IStaticHoCreator>>
{
    std::vector<std::unique_ptr<IStaticHoCreator>> creators;
    creators.push_back(std::make_unique<CarrierCreator>());
    creators.push_back(std::make_unique<BeamCreator>());
    return creators;
}

auto ToString(TransactionType type) -> const char*
{
    switch (type)
    {
        case TransactionType::CreateCarrier:
            return "CreateCarrier";
        case TransactionType::CreateBeam:
            return "CreateBeam";
        default:
            return "Unknown";
    }
}

void Run()
{
    std::cout << "[Abstract Factory]\n";
    auto creators = CreateStartupCreators();
    for (const auto& creator : creators)
    {
        std::cout << "transaction=" << ToString(creator->GetTransactionType()) << "\n";
        auto objects = creator->MakeObjects();
        for (const auto& object : objects)
        {
            std::cout << "  object=" << object->name << ", payload=" << object->payload << "\n";
        }
    }
    std::cout << "\n";
}

} // namespace abstract_factory

int main()
{
    try
    {
        simple_factory::Run();
        abstract_factory::Run();
    }
    catch (const std::exception& ex)
    {
        std::cerr << "factory_demo failed: " << ex.what() << '\n';
        return 1;
    }

    return 0;
}