#include "Main.h"
#include "Tools.h"
#include "cpplinq.hpp"
#include "tinyformat.h"

#include "Logger.hpp"

#include "IGenerator.hpp"

#include "ObjectsStore.hpp"
#include "NamesStore.hpp"
#include "Package.hpp"
#include "NameValidator.hpp"

#include "PrintHelper.hpp"
#include "../Dobby/dobby.h"

extern IGenerator* generator;

void Dump(std::string path)
{
	{

		std::ofstream o(path + "/" + "NamesDump.txt");
		tfm::format(o, "Address: %P\n\n", NamesStore::GetAddress());

		for (auto name : NamesStore())
		{
			tfm::format(o, "[%06i] %s\n", name.Index, name.NamePrivate);
		}
		}
		{
		std::ofstream o(path + "/" + "ObjectsDump.txt");
		tfm::format(o, "Address: %P\n\n", ObjectsStore::GetAddress());

		for (auto obj : ObjectsStore())
		{
			tfm::format(o, "[%06i] %-100s 0x%P\n", obj.GetIndex(), obj.GetFullName(), obj.GetAddress());
		}
	}
}




void SaveSDKHeader(std::string path, const std::unordered_map<UEObject, bool>& processedObjects, const std::vector<std::unique_ptr<Package>>& packages)
{
    std::ofstream os(path + "/" +  "SDK.hpp");

    os << "#pragma once\n\n"
        << tfm::format("// %s (%s) SDK Gen by Sanjay_Src (@SrcEsp) \n", generator->GetGameName(), generator->GetGameVersion());

    //Includes
    os << "#include <set>\n";
    os << "#include <string>\n";
    for (auto&& i : generator->GetIncludes())
    {
        os << "#include " << i << "\n";
    }

    {
        {
            std::ofstream os2(path + "/SDK" + "/" + tfm::format("%s_Basic.hpp", generator->GetGameNameShort()));
            std::vector<std::string> incs = {
            "<iostream>",
            "<string>",
            "<unordered_set>",
            "<codecvt>"
            };
            PrintFileHeader(os2, incs, true);
            
            os2 << generator->GetBasicDeclarations() << "\n";

            PrintFileFooter(os2);

            os << "\n#include \"SDK/" << tfm::format("%s_Basic.hpp", generator->GetGameNameShort()) << "\"\n";
        }
        {
            std::ofstream os2(path + "/SDK" +  "/" +  tfm::format("%s_Basic.cpp", generator->GetGameNameShort()));

            PrintFileHeader(os2, { "\"../SDK.hpp\"" }, false);

            os2 << generator->GetBasicDefinitions() << "\n";

            PrintFileFooter(os2);
        }
    }

    using namespace cpplinq;

    //check for missing structs
    const auto missing = from(processedObjects) >> where([](auto&& kv) { return kv.second == false; });
    if (missing >> any())
    {
        std::ofstream os2(path + "/SDK" + "/" + tfm::format("%s_MISSING.hpp", generator->GetGameNameShort()));

        PrintFileHeader(os2, true);  
		
		for (auto&& s : missing >> select([](auto&& kv) { return kv.first.template Cast<UEStruct>(); }) >> experimental::container())
        {
            os2 << "// " << s.GetFullName() << "\n// ";
            os2 << tfm::format("0x%04X\n", s.GetPropertySize());

            os2 << "struct " << MakeValidName(s.GetNameCPP()) << "\n{\n";
            os2 << "\tunsigned char UnknownData[0x" << tfm::format("%X", s.GetPropertySize()) << "];\n};\n\n";
        }

        PrintFileFooter(os2);

        os << "\n#include \"SDK/" << tfm::format("%s_MISSING.hpp", generator->GetGameNameShort()) << "\"\n";
    }

    os << "\n";

    for (auto&& package : packages)
    {
        os << R"(#include "SDK/)" << GenerateFileName(FileContentType::Structs, *package) << "\"\n";
        os << R"(#include "SDK/)" << GenerateFileName(FileContentType::Classes, *package) << "\"\n";
        if (generator->ShouldGenerateFunctionParametersFile())
        {
            os << R"(#include "SDK/)" << GenerateFileName(FileContentType::FunctionParameters, *package) << "\"\n";
        }
    }
    
}

/// <summary>
/// Process the packages.
/// </summary>
/// <param name="path">The path where to create the package files.</param>
void ProcessPackages(std::string path)
{
    using namespace cpplinq;

    const auto sdkPath = path + "/SDK";
    mkdir(sdkPath.c_str(), 0777);
    
    std::vector<std::unique_ptr<Package>> packages;

    std::unordered_map<UEObject, bool> processedObjects;

    auto packageObjects = from(ObjectsStore())
        >> select([](auto&& o) { return o.GetPackageObject(); })
        >> where([](auto&& o) { return o.IsValid(); })
        >> distinct()
        >> to_vector();

    for (auto obj : packageObjects)
    {
        auto package = std::make_unique<Package>(obj);

        package->Process(processedObjects);
        if (package->Save(sdkPath))
        {
            Package::PackageMap[obj] = package.get();

            packages.emplace_back(std::move(package));
        }
    }

    if (!packages.empty())
    {
      
        const PackageDependencyComparer comparer;
        for (auto i = 0u; i < packages.size() - 1; ++i)
        {
            for (auto j = 0u; j < packages.size() - i - 1; ++j)
            {
                if (!comparer(packages[j], packages[j + 1]))
                {
                    std::swap(packages[j], packages[j + 1]);
                }
            }
        }
    }

    SaveSDKHeader(path, processedObjects, packages);
    SDKManager::SaveSDK(path,gameShortName);
}



void DumpCoreUObjectSizes(std::string path)
{
    UEObject corePackage;

    for (auto obj : ObjectsStore())
    {
        const auto package = obj.GetPackageObject();

        if (package.IsValid() && package.GetFullName() == "Package CoreUObject")
        {
            corePackage = package;
        }
    }

    std::ofstream o(path + "/" + "CoreUObjectInfo.txt");

    if (!corePackage.IsValid()) return;

    for (auto obj : ObjectsStore())
    {
        if (obj.GetPackageObject() == corePackage && obj.IsA<UEClass>())
        {
            auto uclass = obj.Cast<UEClass>();

            if (obj.GetName().find("Default__") == 0) continue;

            tfm::format(o, "class %-35s", uclass.GetNameCPP());

            auto superclass = uclass.GetSuper();

            if (superclass.IsValid())
            {
                tfm::format(o, ": public %-25s ", superclass.GetNameCPP());

                tfm::format(o, "// 0x%02X (0x%02X - 0x%02X) \n", uclass.GetPropertySize() - superclass.GetPropertySize(), superclass.GetPropertySize(), uclass.GetPropertySize());
            }
            else
            {
                tfm::format(o, "%-35s// 0x%02X (0x00 - 0x%02X) \n", "", uclass.GetPropertySize(), uclass.GetPropertySize());
            }
        }
    }


}








void* main_thread (void *)
{
    LOGI("Attaching Dumper...");

sleep(7);
    uintptr_t UE4 = Tools::GetBaseAddress("libUE4.so");
    while (!UE4) {
        UE4 = Tools::GetBaseAddress("libUE4.so");
        sleep(1);
    }
    LOGI("libUE4 Base Address; %zu",UE4);

    if (!ObjectsStore::Initialize())
    {
        return 0;
    }
    LOGI("ObjectsStore::Initialized...");
    if (!NamesStore::Initialize())
    {
        return 0;
    }
    LOGI("NamesStore::Initialized...");
    if (!generator->Initialize())
    {
        return 0;
    }
    LOGI("Generator::Initialized...");

    


    std::string outputDirectory = generator->GetOutputDirectory(pkgName);

    outputDirectory += "/" + generator->GetGameNameShort() + "_(v" + generator->GetGameVersion() + ")_32Bit";


    mkdir(outputDirectory.c_str(), 0777);
    LOGI("Directories Created...");
    LOGI("Dumping Offsets...");

    std::ofstream log(outputDirectory + "/Offsets_Dump.txt");

    auto ProcessEvent_Offset = Tools::FindPattern("libUE4.so", "64 00 9F E5 01 50 A0 E1 00 10 96 E5 02 40 A0 E1");
    if (ProcessEvent_Offset) {
        Logger::SetStream(&log);
        Logger::Log("#define ProcessEvent_Offset 0x%p", ProcessEvent_Offset - 0xc -UE4);
    } else {
        Logger::Log("Failed top search ProcessEvent_Offset!");
    }

    auto GNames_Desi_Offset = Tools::FindPattern("libUE4.so", "60 10 80 E5 64 00 80 E2 A3 E5 3E EB 18 0E 9F E5");
      if (GNames_Desi_Offset) {
        Logger::Log("#define GNames_Desi 0x%p", GNames_Desi_Offset - UE4);
      } else {
        Logger::Log("Failed to search GNames_Desi pattern!");
      }
      
  

    auto GUObjectArray_Offset = Tools::FindPattern("libUE4.so", "?? ?? ?? E5 1F 01 C2 E7 04 00 84 E5 00 20 A0 E3");
    if (GUObjectArray_Offset) {
        GUObjectArray_Offset += *(uintptr_t *) ((GUObjectArray_Offset + *(uint8_t *) (GUObjectArray_Offset) + 0x8)) + 0x18;
        Logger::Log("#define GUObject_Offset 0x%p", GUObjectArray_Offset - UE4);
    } else {
        Logger::Log("Failed to search GUObject_Offset pattern!");
    }


    auto GNames_Offset = Tools::FindPattern("libUE4.so", "E0 01 9F E5 00 00 8F E0 30 70 90 E5");
    if (GNames_Offset) {
    Logger::Log("#define GNames_Offset 0x%p", GNames_Offset - 0x1F8 - UE4);
    } else {
        Logger::Log("Failed to search GNames pattern!");
    }

    auto GNative_Offset = Tools::FindPattern("libUE4.so", "20 01 00 00 9B 03 00 00 24 03 00 00");
    if (GNative_Offset) {
        GNative_Offset = GNative_Offset - 0x7C28 - UE4;
        Logger::Log("#define GNativeAndroidApp_Offset 0x%p ", GNative_Offset);
    } else {
        Logger::Log("Failed to search GNative_OffsetAndroidApp pattern!");
    }
    
    auto GetActorArray = Tools::FindPattern("libUE4.so", "8C 00 9F E5 01 60 A0 E1 00 00 9F E7 00 00 50 E3");
    if (GetActorArray) {
        Logger::SetStream(&log);
        Logger::Log("#define GetActorArray 0x%p", GetActorArray + 0xB8 - UE4);
    } else {
        Logger::Log("Failed top search GetActorArray!");
    }


    auto CanvasMap_Offset = Tools::FindPattern("libUE4.so", "?? ?? ?? E5 24 10 4B E2 18 40 0B E5 00 20 A0 E3");
    if (CanvasMap_Offset) {
        CanvasMap_Offset += *(uintptr_t *) ((CanvasMap_Offset + *(uint8_t *) (CanvasMap_Offset) + 0x8)) + 0x1C;
        Logger::Log("#define CanvasMap_Offset: 0x%p", CanvasMap_Offset - UE4);
    } else {
        Logger::Log("Failed to search CanvasMap pattern!");
    }

    Logger::Log(" ");
    Logger::Log(" ");
    Logger::Log("Sdk Generated By :  Telegram @Sanjay_Src ");
    
    std::ofstream log2(outputDirectory + "/Generator.log");
    Logger::SetStream(&log2);

    Logger::Log("Cheking LOGs");

    Logger::Log("Dumping GNames & GObjects...");
    LOGI("Dumping GNames & GObjects...");
    Dump(outputDirectory);

    Logger::Log("Dumping CoreUobject Infos...");
    LOGI("Dumping CoreUobject Infos...");
    DumpCoreUObjectSizes(outputDirectory);

    const auto begin = std::chrono::system_clock::now();

    Logger::Log("Dumping SDK...");
    LOGI("Dumping SDK...");
    ProcessPackages(outputDirectory);

    Logger::Log("Finished, took %d seconds.", std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now() - begin).count());


    return 0;
}


extern "C"
JNIEXPORT int JNI_OnLoad(JavaVM* vm, void* reserved) {
    JNIEnv* env;
    if (vm->GetEnv(reinterpret_cast<void**>(&env), JNI_VERSION_1_6) != JNI_OK) {
        return JNI_ERR;
    }
    
     Media_Folder = "/storage/emulated/0/Android/media/" + pkgName;
    
    struct stat info;
    if( stat( Media_Folder.c_str(), &info ) != 0 ){
    LOGE( "cannot access %s\n", Media_Folder.c_str() );
    mkdir(Media_Folder.c_str(), 0777);
    }

   pthread_t m_thread;
   pthread_create(&m_thread, 0, main_thread, 0);
    
   return JNI_VERSION_1_6;
}

extern "C"
{
void __attribute__ ((visibility ("default"))) OnLoad() 
{
    
    Media_Folder = "/storage/emulated/0/Android/media/" + pkgName;
    
    struct stat info;
    if( stat( Media_Folder.c_str(), &info ) != 0 ){
    LOGE( "cannot access %s\n", Media_Folder.c_str() );
    mkdir(Media_Folder.c_str(), 0777);
    }
    
      pthread_t thread;
      pthread_create(&thread, 0, main_thread, 0);
      
}
}

