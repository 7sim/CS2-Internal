// used: [win] winapi
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

#include "config.h"
// used: getworkingpath
#include "../../helpers/crt/crt.h"
#include "../../helpers/binary/binary.h"
#include "../../helpers/crt/crt.h"
#define CS_CONFIGURATION_FILE_EXTENSION L".bin"
#include <filesystem>
#include <iostream>
#include <iomanip>
#include <cstdio>   // For fopen, fwrite, fclose
#include "..\..\cheats\skins\inventory.h"
#include "..\..\cheats\skins\inventory_manager.h"
#include "..\..\cheats\skins\skins.h"

// default configurations working path
static wchar_t wszConfigurationsPath[MAX_PATH];
namespace fs = std::filesystem;

#include <string>  // For std::wstring and std::string conversions
#include "../../cheats/skins/skins.h"
#include "../render/ui/menu.h"
#include <VTexParser.h>
#include <unordered_set>  // Include for std::unordered_set

// Define a set to track UUIDs that have already been added
static std::unordered_set<std::string> added_uuids;

#include <unordered_set>  // Include for std::unordered_set
 std::vector< KeyBind_t>  m_key_binds;
// Define a set to track ulIDs that have already been added
static std::unordered_set<uint64_t> added_ulIDs; 
bool C::SaveSkinToJson(const std::size_t idx) {

	const wchar_t* wszFileName = vecFileNames[idx];
	wchar_t wszFilePath[MAX_PATH];
	c_run_time::StringCopy(wszFilePath, wszConfigurationsPath);  // Copy configurations path first

	// Append file name with .json suffix
	std::wstring fileNameWithSuffix = wszFileName;
	fileNameWithSuffix += L".json";

	// Concatenate full file path
	c_run_time::StringCat(wszFilePath, fileNameWithSuffix.c_str());

	// Open the file for writing (this will overwrite the existing file)
	FILE* file = nullptr;
	errno_t err = _wfopen_s(&file, wszFilePath, L"w");  // Open file for writing
	if (err != 0 || file == nullptr) {
		L_PRINT(LOG_WARNING) << "Failed to open file for writing.";
		return false;
	}

	// Create a new JSON array to save the new skin configurations
	json newJsonArray = json::array();

	// Add all items from added_item_list to the new JSON array
	for (auto& econ : added_item_list) {
		json skinJson;
		econ.to_json(skinJson);  // Convert each econ to JSON format
		newJsonArray.push_back(skinJson);  // Add it to the new array
	}

	// Write the new JSON data to the file (this will overwrite the entire file)
	try {
		std::string jsonData = newJsonArray.dump(4);  // Dump JSON with indentation
		fwrite(jsonData.c_str(), sizeof(char), jsonData.size(), file);
		fclose(file);
		return true;
	}
	catch (const std::exception& e) {
		L_PRINT(LOG_WARNING) << "Error saving JSON: " << e.what();
		if (file) fclose(file);
		return false;
	}
}
std::string generate_random_strisng(int length) {
	const std::string chars = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
	std::string result;
	result.reserve(length);
	for (int i = 0; i < length; ++i) {
		result += chars[rand() % chars.size()];
	}
	return result;
}


ID3D11ShaderResourceView* get_image(std::string path_image) {

	auto vtex = VTexParser::Load(path_image.c_str());

	if (vtex.data.empty())
		return nullptr;

	ID3D11ShaderResourceView* image = nullptr;
	Items::skins->create_texture(vtex.w, vtex.h, reinterpret_cast<unsigned char*>(vtex.data.data()), &image);
	return image;

}


// Struct to store equip data
struct EquipData {
	uint64_t uuid;
	int team;
	int slot;
};

// Vector to store items to be equipped
std::vector<EquipData> equipItems;

// Variable to store the last equip time
std::chrono::steady_clock::time_point lastEquipTime = std::chrono::steady_clock::now();

// Time delay between equip actions (200ms to 300ms)
const std::chrono::milliseconds equipDelayMin(200);
const std::chrono::milliseconds equipDelayMax(300);

// Function to process the equip items
void equip_items(CCSInventoryManager* inventory_manager) {
	auto now = std::chrono::steady_clock::now();
	auto elapsedTime = std::chrono::duration_cast<std::chrono::milliseconds>(now - lastEquipTime);

	// Check if enough time has passed for the next equip action
	if (elapsedTime >= equipDelayMin) {
		// Process the next equip item
		if (!equipItems.empty()) {
			EquipData equipData = equipItems.front();
			equipItems.erase(equipItems.begin()); // Remove the equipped item

			// Equip the item based on the team and slot
			if (equipData.team == 4) {
				inventory_manager->EquipItemInLoadout(3, equipData.slot, equipData.uuid);
				inventory_manager->EquipItemInLoadout(2, equipData.slot, equipData.uuid);
			}
			else {
				inventory_manager->EquipItemInLoadout(equipData.team, equipData.slot, equipData.uuid);
			}

			// Log the equip
			L_PRINT(LOG_INFO) << "Equipped item with UUID: " << equipData.uuid;

			// Update the last equip time
			lastEquipTime = now;

			// Optional: Random delay between 200ms and 300ms (you can adjust the range)
			std::this_thread::sleep_for(std::chrono::milliseconds(200 + rand() % 101)); // Random delay
		}
	}
}
bool C::LoadSkinFromJson(const std::size_t idx)
{

	auto inventory_manager = CCSInventoryManager::GetInstance();
	if (!inventory_manager) {
		L_PRINT(LOG_WARNING) << "[config] couldn't get inventory manager instance.";
		return false;
	}

	auto inventory = CCSPlayerInventory::GetInstance();
	if (!inventory) {
		L_PRINT(LOG_WARNING) << "[config] couldn't get player inventory instance.";
		return false;
	}

	const wchar_t* wszFileName = vecFileNames[idx];
	wchar_t wszFilePath[MAX_PATH];
	c_run_time::StringCopy(wszFilePath, wszConfigurationsPath);  // Copy configurations path first

	// Append file name with .json suffix
	std::wstring fileNameWithSuffix = wszFileName;
	fileNameWithSuffix += L".json";

	// Concatenate full file path
	c_run_time::StringCat(wszFilePath, fileNameWithSuffix.c_str());

	FILE* file = nullptr;
	errno_t err = _wfopen_s(&file, wszFilePath, L"rb");  // Open file for reading in binary mode

	if (err != 0 || file == nullptr) {
		L_PRINT(LOG_WARNING) << "Failed to open file for reading.";
		return false;
	}

	// Determine file size
	fseek(file, 0, SEEK_END);
	long fileSize = ftell(file);
	fseek(file, 0, SEEK_SET);

	// Read entire file content into a buffer
	std::string fileContent(fileSize, '\0');
	fread(&fileContent[0], sizeof(char), fileSize, file);
	fclose(file);
	if (fileContent.empty()) {
		_log(LOG_WARNING) << "empty buffer skipping file";
		return false;
	}

	// Parse JSON from file content
	json j = json::parse(fileContent);

	// Ensure j is an array
	if (!j.is_array()) {
		L_PRINT(LOG_WARNING) << "JSON content is not an array.";
		return false;
	}

	// Iterate over each skin in the array
	for (auto& skinJson : j) {
		EconItem_t skin;
		skin.from_json(skinJson);
		auto str = generate_random_strisng(4);
		bool found_duplicated = false;

		if (!added_skins.empty()) {
			// Check if the skin already exists in added_skins
			for (const auto& addedSkin : added_skins) {
				if (addedSkin.path_image == skin.image) {
					found_duplicated = true;
					continue;
				}
			}
		}

		if (found_duplicated)
			continue;

		auto pItem = CEconItem::CreateInstance();
		if (!pItem) {
			L_PRINT(LOG_WARNING) << "Failed to create CEconItem instance.";
			continue;
		}

		static std::string old_str = str.c_str();
		str = std::string("##" + old_str);
		skin.name = str.c_str();
		auto highestIDs = inventory->GetHighestIDs();

		pItem->m_ulID = highestIDs.first + 1;
		pItem->m_unInventory = highestIDs.second + 1;
		pItem->m_unAccountID = uint32_t(inventory->GetOwner().m_id);
		pItem->m_unDefIndex = skin.unDefIndex;
		pItem->m_nRarity = std::clamp(skin.nRarity + skin.nRarity - 1, 0, (skin.nRarity == 7) ? 7 : 6);

		pItem->SetPaintKit(skin.flPaintKit);
		pItem->SetPaintSeed(skin.flPaintSeed);
		pItem->SetPaintWear(skin.flPaintWear);

		if (skin.nStatTrak >= 0) {
			pItem->SetStatTrak(skin.nStatTrak);
			pItem->SetStatTrakType(0);

			// Applied automatically on knives.
			if (pItem->m_nQuality != IQ_UNUSUAL)
				pItem->m_nQuality = IQ_STRANGE;
		}

		int team = skin.team;
		int slot = skin.slot;

		int econ_key_chain_id = skin.econ_key_chain_i;
		int econ_sticker_id_1 = skin.econ_sticker_id_1;
		int econ_sticker_id_2 = skin.econ_sticker_id_2;
		int econ_sticker_id_3 = skin.econ_sticker_id_3;
		int econ_sticker_id_4 = skin.econ_sticker_id_4;
		float sticker_rot = 1.f;
		float sticker_wear = 0.f;
		float key_chains_addon = 0.f;


		// has key chains ?
		if (econ_key_chain_id != -1) {
			pItem->SetDynamicAttributeValue(299, &econ_key_chain_id);
			pItem->SetDynamicAttributeValue(300, &key_chains_addon);
			pItem->SetDynamicAttributeValue(301, &key_chains_addon);
			pItem->SetDynamicAttributeValue(302, &key_chains_addon);
		}

		// has stickers ? 
		for (int cur_sticker_index = 0; cur_sticker_index < 4; cur_sticker_index++) {
			if (cur_sticker_index == 0)
				continue;

			// set sticker id based on slot 
			int cur_sticker = 0;

			if (cur_sticker_index == 1)
				cur_sticker = econ_sticker_id_1;
			else if (cur_sticker_index == 2)
				cur_sticker = econ_sticker_id_2;
			else if (cur_sticker_index == 3)
				cur_sticker = econ_sticker_id_3;
			else if (cur_sticker_index == 4)
				cur_sticker = econ_sticker_id_4;

			// check if cur slot sticker is not invalid or nulled 
			if (cur_sticker <= 0 || cur_sticker == -1)
				continue;

			// if all good set stickers
			pItem->SetDynamicAttributeValue(113 + cur_sticker_index * sizeof(uint32_t), &cur_sticker);
			pItem->SetDynamicAttributeValue(114 + cur_sticker_index * sizeof(uint32_t), &sticker_wear);
			pItem->SetDynamicAttributeValue(115 + cur_sticker_index * sizeof(uint32_t), &sticker_rot);
			pItem->SetDynamicAttributeValue(116 + cur_sticker_index * sizeof(uint32_t), &sticker_rot);
			_log(LOG_INFO) << " added sticker on slot: " << cur_sticker_index << " id:" << cur_sticker;	

			// skip keep looking
			continue;
		}


		L_PRINT(LOG_WARNING) << "Loading C_EconItemView data| ulID: " << skin.ulID << " | name: " << skin.name.c_str() << " defidx: " << skin.unDefIndex << " | slot: " << skin.slot << " | team: " << skin.team;

		// Add item to inventory
		if (inventory->AddEconItem(pItem)) {
			Items::skins->add(pItem);
		}

		// Store the equip information in the external vector
		equipItems.push_back({ pItem->m_ulID, team, slot });

		const char* name = str.c_str();

		EconItem_t econItem = {
			   pItem->m_ulID,
			   pItem->m_unInventory,
			   pItem->m_unAccountID,
			   pItem->m_unDefIndex,
			   pItem->m_nRarity,
			   skin.flPaintKit,
			   (float)skin.flPaintSeed,
			   (float)skin.flPaintWear,
			   0,
			   0,
			   pItem->m_nQuality,
			   skin.image.c_str(),
			   true,
			   skin.name,
			   team,
			   slot,
			   econ_key_chain_id,
			   econ_sticker_id_1,
			   econ_sticker_id_2,
			   econ_sticker_id_3,
			   econ_sticker_id_4
		};

		added_item_list.emplace_back(econItem);


		c_paintKits paintkit;
		paintkit.id = skin.ulID;
		paintkit.rarity = skin.nRarity;
		paintkit.name = str.c_str();
		paintkit.path_image = skin.image.c_str();

		for (auto& econ : added_skins) {
			if (econ.path_image == paintkit.path_image) {
				return false;
			}
		}

		L_PRINT(LOG_INFO) << "added CEconItem to Config system instance: " << paintkit.name << " | " << name;

		auto vtex = VTexParser::Load(skin.image.c_str());

		if (!vtex.data.empty())
			Items::skins->create_texture(vtex.w, vtex.h, reinterpret_cast<unsigned char*>(vtex.data.data()), &paintkit.image);
		paintkit.item_id = pItem->m_ulID;

		added_skins.push_back(paintkit);
		L_PRINT(LOG_INFO) << "added skins: " << added_skins.size() << " name:" << paintkit.name;
	}
	
	if (equipItems.empty())
		return false;

	// Equip items outside the loop after everything is loaded and added
	for (const auto& equipData : equipItems) {
		equip_items(inventory_manager);
	}

	return true;
}
#pragma region config_user_data_type

std::size_t C::UserDataType_t::GetSerializationSize() const
{
	std::size_t nTotalDataSize = 0U;

	for (const UserDataMember_t& member : vecMembers)
		nTotalDataSize += sizeof(uint32_t[2]) + member.nDataSize;

	return nTotalDataSize;
}

#pragma endregion

#pragma region config_variable_object

void C::VariableObject_t::SetStorage(const void* pValue)
{
	// check is available to store value in the local storage
	if (this->nStorageSize <= sizeof(this->storage.uLocal))
	{
		memset(&this->storage.uLocal, 0U, sizeof(this->storage.uLocal));
		memcpy(&this->storage.uLocal, pValue, this->nStorageSize);
	}
	// otherwise use heap memory to store it
	else
	{
		debug(this->storage.pHeap != nullptr); // tried to access non allocated storage

		memset(this->storage.pHeap, 0U, this->nStorageSize);
		memcpy(this->storage.pHeap, pValue, this->nStorageSize);
	}
}

std::size_t C::VariableObject_t::GetSerializationSize() const
{
	std::size_t nSerializationSize = this->nStorageSize;

	// denote a custom serialization size when it different from the storage size
	switch (this->uTypeHash)
	{
	// lookup for array data type
	case hash_32_fnv1a_const("bool[]"):
	case hash_32_fnv1a_const("int[]"):
	case hash_32_fnv1a_const("unsigned int[]"):
	case hash_32_fnv1a_const("float[]"):
	case hash_32_fnv1a_const("char[][]"):
		// arrays also serialize their size
		nSerializationSize += sizeof(std::size_t);
		break;
	// lookup for user-defined data type
	default:
	{
		for (const UserDataType_t& userType : vecUserTypes)
		{
			if (userType.uTypeHash == this->uTypeHash)
			{
				nSerializationSize = sizeof(std::size_t) + userType.GetSerializationSize();
				break;
			}
		}
		break;
	}
	}

	return nSerializationSize;
}

#pragma endregion
#include <shlobj_core.h>

bool get_path(wchar_t* wszDestination)
{
	bool bSuccess = false;
	PWSTR wszPathToDocuments = nullptr;

	// get path to user documents
	if (SUCCEEDED(::SHGetKnownFolderPath(FOLDERID_Documents, KF_FLAG_CREATE, nullptr, &wszPathToDocuments)))
	{
		c_run_time::StringCat(c_run_time::StringCopy(wszDestination, wszPathToDocuments), xorstr_(L"\\.cs2\\"));
		bSuccess = true;
		// create directory if it doesn't exist
		if (!CreateDirectoryW(wszDestination, nullptr))
		{
			if (::GetLastError() != ERROR_ALREADY_EXISTS)
			{
				bSuccess = false;
			}
		}
	}
	::CoTaskMemFree(wszPathToDocuments);

	return bSuccess;
}
bool C::Setup(const wchar_t* wszDefaultFileName)
{

	if (!get_path(wszConfigurationsPath))
		return false;

	c_run_time::StringCat(wszConfigurationsPath, xorstr_(L"settings\\"));

	// create directory if it doesn't exist
	if (!::CreateDirectoryW(wszConfigurationsPath, nullptr))
	{
		if (::GetLastError() != ERROR_ALREADY_EXISTS)
		{
			return false;
		}
	}
	// @note: define custom data types we want to serialize
	AddUserType(hash_32_fnv1a_const("KeyBind_t"),
	{ 
		UserDataMember_t{ hash_32_fnv1a_const("uKey"), hash_32_fnv1a_const("unsigned int"), &KeyBind_t::uKey },
		UserDataMember_t{ hash_32_fnv1a_const("nMode"), hash_32_fnv1a_const("int"), &KeyBind_t::nMode } 
	});

	AddUserType(hash_32_fnv1a_const("ColorPickerVar_t"),
	{
		UserDataMember_t{ hash_32_fnv1a_const("bRainbow"), hash_32_fnv1a_const("bool"), &ColorPickerVar_t::bRainbow },
		UserDataMember_t{ hash_32_fnv1a_const("flRainbowSpeed"), hash_32_fnv1a_const("float"), &ColorPickerVar_t::flRainbowSpeed },
		UserDataMember_t{ hash_32_fnv1a_const("colPrimary"), hash_32_fnv1a_const("Color_t"), &ColorPickerVar_t::colValue },
	});

	AddUserType(hash_32_fnv1a_const("TextOverlayVar_t"), 
	{ 
		UserDataMember_t{ hash_32_fnv1a_const("bEnable"), hash_32_fnv1a_const("bool"), &TextOverlayVar_t::bEnable },
		UserDataMember_t{ hash_32_fnv1a_const("flThickness"), hash_32_fnv1a_const("float"), &TextOverlayVar_t::flThickness },
		UserDataMember_t{ hash_32_fnv1a_const("colPrimary"), hash_32_fnv1a_const("Color_t"), &TextOverlayVar_t::colPrimary },
		UserDataMember_t{ hash_32_fnv1a_const("colOutline"), hash_32_fnv1a_const("Color_t"), &TextOverlayVar_t::colOutline }
	});

	AddUserType(hash_32_fnv1a_const("FrameOverlayVar_t"),
	{
		UserDataMember_t{ hash_32_fnv1a_const("bEnable"), hash_32_fnv1a_const("bool"), &FrameOverlayVar_t::bEnable },
		UserDataMember_t{ hash_32_fnv1a_const("flThickness"), hash_32_fnv1a_const("float"), &FrameOverlayVar_t::flThickness },
		UserDataMember_t{ hash_32_fnv1a_const("flRounding"), hash_32_fnv1a_const("float"), &FrameOverlayVar_t::flRounding },
		UserDataMember_t{ hash_32_fnv1a_const("colPrimary"), hash_32_fnv1a_const("Color_t"), &FrameOverlayVar_t::colPrimary },
		UserDataMember_t{ hash_32_fnv1a_const("colOutline"), hash_32_fnv1a_const("Color_t"), &FrameOverlayVar_t::colOutline } 
	});

	AddUserType(hash_32_fnv1a_const("BarOverlayVar_t"),
	{ 
		UserDataMember_t{ hash_32_fnv1a_const("bEnable"), hash_32_fnv1a_const("bool"), &BarOverlayVar_t::bEnable },
		UserDataMember_t{ hash_32_fnv1a_const("bGradient"), hash_32_fnv1a_const("bool"), &BarOverlayVar_t::bGradient },
		UserDataMember_t{ hash_32_fnv1a_const("bUseFactorColor"), hash_32_fnv1a_const("bool"), &BarOverlayVar_t::bUseFactorColor },
		UserDataMember_t{ hash_32_fnv1a_const("flThickness"), hash_32_fnv1a_const("float"), &BarOverlayVar_t::flThickness },
		UserDataMember_t{ hash_32_fnv1a_const("colPrimary"), hash_32_fnv1a_const("Color_t"), &BarOverlayVar_t::colPrimary },
		UserDataMember_t{ hash_32_fnv1a_const("colSecondary"), hash_32_fnv1a_const("Color_t"), &BarOverlayVar_t::colSecondary },
		UserDataMember_t{ hash_32_fnv1a_const("colBackground"), hash_32_fnv1a_const("Color_t"), &BarOverlayVar_t::colBackground },
		UserDataMember_t{ hash_32_fnv1a_const("colOutline"), hash_32_fnv1a_const("Color_t"), &BarOverlayVar_t::colOutline } 
	});

	// create default configuration
	if (!CreateFile(wszDefaultFileName))
		return false;

	// store existing configurations list
	Refresh();

	return true;
}

#pragma region config_main

void C::Refresh()
{
	// clear and free previous stored file names
	vecFileNames.clear();

	// make configuration files path filter
	wchar_t wszPathFilter[MAX_PATH];
	c_run_time::StringCat(c_run_time::StringCopy(wszPathFilter, wszConfigurationsPath), (L"*" CS_CONFIGURATION_FILE_EXTENSION));

	// iterate through all files with our filter
	WIN32_FIND_DATAW findData;
	if (const HANDLE hFindFile = ::FindFirstFileW(wszPathFilter, &findData); hFindFile != INVALID_HANDLE_VALUE)
	{
		do
		{
			vecFileNames.push_back(new wchar_t[c_run_time::StringLength(findData.cFileName) + 1U]);
			c_run_time::StringCopy(vecFileNames.back(), findData.cFileName);

		} while (::FindNextFileW(hFindFile, &findData));

		::FindClose(hFindFile);
	}
}

void C::AddUserType(const uint32_t uTypeHash, const std::initializer_list<UserDataMember_t> vecUserMembers)
{
	if (vecUserMembers.size() == 0U)
		return;

	UserDataType_t userDataType;
	userDataType.uTypeHash = uTypeHash;

	for (const auto& userDataMember : vecUserMembers)
		userDataType.vecMembers.push_back(userDataMember);

	vecUserTypes.emplace_back(c_run_time::Move(userDataType));
}


void C::SerializeEconItem(C::VariableObject_t& variable, const EconItem_t& item) {
	variable.uTypeHash = hash_32_fnv1a_const("EconItem_t");
	variable.nStorageSize = sizeof(EconItem_t);
	variable.SetStorage(&item);
}


bool C::SaveEconItem(const std::size_t nFileIndex, const EconItem_t& item) {
	C::VariableObject_t variable = {0, 0, 0};
	C::SerializeEconItem(variable, item); 

	const wchar_t* wszFileName = vecFileNames[nFileIndex];
	wchar_t wszFilePath[MAX_PATH];
	c_run_time::StringCat(c_run_time::StringCopy(wszFilePath, wszConfigurationsPath), wszFileName);

	if (BIN::SaveVariable(wszFilePath, variable))
	{
		return true;
	}

	return false;
}

bool C::LoadEconItem(const std::size_t nFileIndex, const EconItem_t& item) {
	C::VariableObject_t variable = { 0, 0, 0 };

	const wchar_t* wszFileName = vecFileNames[nFileIndex];

	wchar_t wszFilePath[MAX_PATH];
	c_run_time::StringCat(c_run_time::StringCopy(wszFilePath, wszConfigurationsPath), wszFileName);

	if (BIN::LoadVariable(wszFilePath, variable))
	{
		return true;
	}

	return false;
}
bool C::SaveFileVariable(const std::size_t nFileIndex, const VariableObject_t& variable)
{
	const wchar_t* wszFileName = vecFileNames[nFileIndex];

	wchar_t wszFilePath[MAX_PATH];
	c_run_time::StringCat(c_run_time::StringCopy(wszFilePath, wszConfigurationsPath), wszFileName);

	if (BIN::SaveVariable(wszFilePath, variable))
	{
		return true;
	}

	return false;
}

bool C::LoadFileVariable(const std::size_t nFileIndex, VariableObject_t& variable)
{
	const wchar_t* wszFileName = vecFileNames[nFileIndex];

	wchar_t wszFilePath[MAX_PATH];
	c_run_time::StringCat(c_run_time::StringCopy(wszFilePath, wszConfigurationsPath), wszFileName);

	if (BIN::LoadVariable(wszFilePath, variable))
	{ 
		return true;
	}

	return false;
}

bool C::RemoveFileVariable(const std::size_t nFileIndex, const VariableObject_t& variable)
{
	const wchar_t* wszFileName = vecFileNames[nFileIndex];

	wchar_t wszFilePath[MAX_PATH];
	c_run_time::StringCat(c_run_time::StringCopy(wszFilePath, wszConfigurationsPath), wszFileName);

	if (BIN::RemoveVariable(wszFilePath, variable))
	{
		return true;
	}

	return false;
}

bool C::CreateFile(const wchar_t* wszFileName)
{
	const wchar_t* wszFileExtension = c_run_time::StringCharR(wszFileName, L'.');

	// get length of the given filename and strip out extension if there any
	const std::size_t nFileNameLength = (wszFileExtension != nullptr ? wszFileExtension - wszFileName : c_run_time::StringLength(wszFileName));
	wchar_t* wszFullFileName = new wchar_t[nFileNameLength + c_run_time::StringLength(CS_CONFIGURATION_FILE_EXTENSION) + 1U];

	// copy filename without extension
	wchar_t* wszFullFileNameEnd = c_run_time::StringCopyN(wszFullFileName, wszFileName, nFileNameLength);
	*wszFullFileNameEnd = L'\0';
	// append correct extension to the filename
	c_run_time::StringCat(wszFullFileNameEnd, xorstr_(CS_CONFIGURATION_FILE_EXTENSION));

	// add filename to the list
	vecFileNames.push_back(wszFullFileName);

	// create and save it by the index
	if (SaveFile(vecFileNames.size() - 1U)){
		return true;
	}

	return false;
}

bool C::SaveFile(const std::size_t nFileIndex)
{
	const wchar_t* wszFileName = vecFileNames[nFileIndex];

	wchar_t wszFilePath[MAX_PATH];
	c_run_time::StringCat(c_run_time::StringCopy(wszFilePath, wszConfigurationsPath), wszFileName);

	if (BIN::SaveFile(wszFilePath))
	{
		return true;
	}

	return false;
}

bool C::LoadFile(const std::size_t nFileIndex)
{
	const wchar_t* wszFileName = vecFileNames[nFileIndex];

	wchar_t wszFilePath[MAX_PATH];
	c_run_time::StringCat(c_run_time::StringCopy(wszFilePath, wszConfigurationsPath), wszFileName);

	if (BIN::LoadFile(wszFilePath))
	{
		return true;
	}

	return false;
}

void C::RemoveFile(const std::size_t nFileIndex)
{
	const wchar_t* wszFileName = vecFileNames[nFileIndex];

	// unable to delete default config
	if (c_run_time::StringCompare(wszFileName, xorstr_(CS_CONFIGURATION_DEFAULT_FILE_NAME  CS_CONFIGURATION_FILE_EXTENSION)) == 0)
	{
		return;
	}

	wchar_t wszFilePath[MAX_PATH];
	c_run_time::StringCat(c_run_time::StringCopy(wszFilePath, wszConfigurationsPath), wszFileName);

	if (::DeleteFileW(wszFilePath))
	{
		// erase and free filename from the list
		vecFileNames.erase(vecFileNames.cbegin() + nFileIndex);

	}
}

#pragma endregion

#pragma region config_get

std::size_t C::GetVariableIndex(const uint32_t uNameHash)
{
	for (std::size_t i = 0U; i < vecVariables.size(); i++)
	{
		if (vecVariables[i].uNameHash == uNameHash)
			return i;
	}

	return cfg_invalidate;
}

#pragma endregion

#pragma region config_user_types
void ColorPickerVar_t::UpdateRainbow()
{
	// @todo: improve + optimize this code
	// progress rainbow color
	if (this->bRainbow)
	{
		const float flTime = static_cast<float>(ImGui::GetTime());
		// create a rainbow color with copied alpha
		float arrRainbowColors[] = {
			sin(flTime * this->flRainbowSpeed) * 0.5f + 0.5f,
			sin(flTime * this->flRainbowSpeed * math::_PI / 3) * 0.5f + 0.5f,
			sin(flTime * this->flRainbowSpeed * math::_PI / 3) * 0.5f + 0.5f,
			this->colValue.Base<COLOR_A>()
		};

		// set the rainbow color
		this->colValue = Color_t::FromBase4(arrRainbowColors);
	}
}
