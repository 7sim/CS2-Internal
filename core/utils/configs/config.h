#pragma once
// used: [stl] vector
#include <vector>
// used: [stl] type_info
#include <typeinfo>
// used: [win] undname_no_arguments
#include <dbghelp.h>

#include "../../sdk/sdk.h"

#include "..\..\..\deps\json\json.h" // include the JSON library

using json = nlohmann::json;
enum Removals_t : unsigned int {
	view_bob = 1,
	flash = 2,
	visual_recoil = 3,
};
struct EconItem_t {
	unsigned long long ulID;
	unsigned int unInventory;
	unsigned int unAccountID;
	unsigned int unDefIndex;
	int nRarity;
	int flPaintKit;
	float flPaintSeed;
	float flPaintWear;
	int nStatTrak;
	int nStatTrakType;
	int nQuality;
	std::string image;
	bool added;
	std::string name;
	int team;
	int slot;
	bool equiped;
	int econ_key_chain_i;
	int	econ_sticker_id_1;
	int	econ_sticker_id_2;
	int	econ_sticker_id_3;
	int	econ_sticker_id_4;
	// Serialize to JSON
	void to_json(json& j) const {
		j = json{
			{"ulID", ulID},
			{"unInventory", unInventory},
			{"unAccountID", unAccountID},
			{"unDefIndex", unDefIndex},
			{"nRarity", nRarity},
			{"flPaintKit", flPaintKit},
			{"flPaintSeed", flPaintSeed},
			{"flPaintWear", flPaintWear},
			{"nStatTrak", nStatTrak},
			{"nStatTrakType", nStatTrakType},
			{"nQuality", nQuality},
			{"image", image},
			{"added", added},
			{"name", name},
			{"team", team},
			{"slot", slot},
			{"econ_key_chain_i", econ_key_chain_i},
			{"econ_sticker_id_1", econ_sticker_id_1},
			{"econ_sticker_id_2", econ_sticker_id_2},
			{"econ_sticker_id_3", econ_sticker_id_3},
			{"econ_sticker_id_4", econ_sticker_id_4}

		};
	}

	// Deserialize from JSON
	void from_json(const json& j) {
		j.at("ulID").get_to(ulID);
		j.at("unInventory").get_to(unInventory);
		j.at("unAccountID").get_to(unAccountID);
		j.at("unDefIndex").get_to(unDefIndex);
		j.at("nRarity").get_to(nRarity);
		j.at("flPaintKit").get_to(flPaintKit);
		j.at("flPaintSeed").get_to(flPaintSeed);
		j.at("flPaintWear").get_to(flPaintWear);
		j.at("nStatTrak").get_to(nStatTrak);
		j.at("nStatTrakType").get_to(nStatTrakType);
		j.at("nQuality").get_to(nQuality);
		j.at("image").get_to(image);
		j.at("added").get_to(added);
		j.at("name").get_to(name);
		j.at("team").get_to(team);
		j.at("slot").get_to(slot);
		j.at("econ_key_chain_i").get_to(econ_key_chain_i);
		j.at("econ_sticker_id_1").get_to(econ_sticker_id_1);
		j.at("econ_sticker_id_2").get_to(econ_sticker_id_2);
		j.at("econ_sticker_id_3").get_to(econ_sticker_id_3);
		j.at("econ_sticker_id_4").get_to(econ_sticker_id_4);
	}
};

#pragma region config_definitions
#define add_cfg(TYPE, NAME, DEFAULT) const std::size_t NAME = C::AddVariable<TYPE>(hash_32_fnv1a_const(#NAME), hash_32_fnv1a_const(#TYPE), DEFAULT);

#define add_adaptive_cfg(TYPE, SIZE, NAME, DEFAULT) const std::size_t NAME = C::AddVariableArray<TYPE[SIZE]>(hash_32_fnv1a_const(#NAME), hash_32_fnv1a_const(#TYPE "[]"), DEFAULT);

#define add_adaptive_array_cfg(TYPE, SIZE, SUBSIZE, NAME, DEFAULT) const std::size_t NAME = C::AddVariableArray<TYPE[SIZE][SUBSIZE]>(hash_32_fnv1a_const(#NAME), hash_32_fnv1a_const(#TYPE "[][]"), DEFAULT);

#define cfg_invalidate static_cast<std::size_t>(-1)

#define cfg_get(TYPE, NAME) C::Get<TYPE>(NAME)
#define cfg_set(TYPE, NAME, VALUE) C::Set<TYPE>(C::GetVariableIndex(hash_32_fnv1a_const(#NAME)), VALUE)

#define cfg_get_adaptive(TYPE, SIZE, NAME, INDEX) C::Get<TYPE[SIZE]>(NAME)[INDEX]
#pragma endregion

#pragma region config_user_types
enum class EKeyBindMode : int
{
	HOLD = 0,
	TOGGLE
};

struct KeyBind_t
{
	constexpr KeyBind_t(const char* szName, const unsigned int uKey = 0U, const int nMode = 0) :
		szName(szName), uKey(uKey), nMode(nMode) { }

	 bool bEnable = false;
	const char* szName = nullptr;
	unsigned int uKey = 0U;
	int nMode =0;
};
extern std::vector< KeyBind_t>  m_key_binds;
struct ColorPickerVar_t
{
	// default constructor
	constexpr ColorPickerVar_t(const Color_t& colValue = Color_t(255, 255, 255), const bool bRainbow = false, const float flRainbowSpeed = 0.5f) :
		colValue(colValue), bRainbow(bRainbow), flRainbowSpeed(flRainbowSpeed) { }

	// @note: other contructors will only construct Color_t object and set rainbow to false and speed to 0.5f

	// 8-bit color constructor (in: [0 .. 255])
	constexpr ColorPickerVar_t(const std::uint8_t r, const std::uint8_t g, const std::uint8_t b, const std::uint8_t a = 255) :
		colValue(r, g, b, a), bRainbow(false), flRainbowSpeed(0.5f) { }

	// 8-bit color constructor (in: [0 .. 255])
	constexpr ColorPickerVar_t(const int r, const int g, const int b, const int a = 255) :
		colValue(r, g, b, a), bRainbow(false), flRainbowSpeed(0.5f) { }

	// 8-bit array color constructor (in: [0.0 .. 1.0])
	explicit constexpr ColorPickerVar_t(const std::uint8_t arrColor[4]) :
		colValue(arrColor), bRainbow(false), flRainbowSpeed(0.5f) { }

	// 32-bit packed color constructor (in: 0x00000000 - 0xFFFFFFFF)
	explicit constexpr ColorPickerVar_t(const ImU32 uPackedColor) :
		colValue(uPackedColor), bRainbow(false), flRainbowSpeed(0.5f) { }

	// 32-bit color constructor (in: [0.0 .. 1.0])
	constexpr ColorPickerVar_t(const float r, const float g, const float b, const float a = 1.0f) :
		colValue(r, g, b, a), bRainbow(false), flRainbowSpeed(0.5f) { }

	

	void UpdateRainbow();

	Color_t colValue = Color_t(255, 255, 255, 255);
	bool bRainbow = false;
	float flRainbowSpeed = 0.5f;
};

/// hold config variables for text component overlay
struct TextOverlayVar_t
{
	constexpr TextOverlayVar_t(const bool bEnable, const bool bIcon, const float flThickness = 1.f, const Color_t& colPrimary = Color_t(255, 255, 255), const Color_t& colOutline = Color_t(0, 0, 0, 155)) :
		bEnable(bEnable), bIcon(bIcon), flThickness(flThickness), colPrimary(colPrimary), colOutline(colOutline) { }

	bool bEnable = false;
	bool bIcon = false;
	float flThickness = 1.f;
	Color_t colPrimary = Color_t(222, 222, 222, 200);
	Color_t colOutline = Color_t(0, 0, 0, 155);
};

/// hold config variables for frame/box component overlay
struct FrameOverlayVar_t
{
	constexpr FrameOverlayVar_t(const bool bEnable, const float flThickness = 1.f, const float flRounding = 0.f, const Color_t& colPrimary = Color_t(255, 255, 255), const Color_t& colOutline = Color_t(0, 0, 0, 155)) :
		bEnable(bEnable), flThickness(flThickness), flRounding(flRounding), colPrimary(colPrimary), colOutline(colOutline) { }

	bool bEnable = false;
	float flThickness = 1.f;

	float flRounding = 0.f;

	Color_t colPrimary = Color_t(255, 255, 255);
	Color_t colOutline = Color_t(0, 0, 0, 155);
};

/// hold config variables for bar component overlay
struct BarOverlayVar_t
{
	constexpr BarOverlayVar_t(const bool bEnable, const bool bGradient = false, const bool bUseFactorColor = false, const float flThickness = 2.2f, const Color_t& colPrimary = Color_t(255, 255, 255), const Color_t& colSecondary = Color_t(255, 255, 255), const Color_t& colBackground = Color_t(5, 5, 5, 155), const Color_t& colOutline = Color_t(0,0, 0, 155), const bool background = true, const bool outline = true) :
		bEnable(bEnable), bGradient(bGradient), bUseFactorColor(bUseFactorColor), flThickness(flThickness), colPrimary(colPrimary), colSecondary(colSecondary), colBackground(colBackground), colOutline(colOutline), bBackground(background), bOutline(outline) { }

	bool bEnable = false;
	bool bGradient = false;
	bool bShowValue  = false;
	bool bOutline	 = false;
	bool bGlowShadow = false;
	bool bBackground = false;
	bool bUseFactorColor = false;
	float flThickness = 1.f;
	
	Color_t colPrimary = Color_t(255, 255, 255);
	Color_t colSecondary = Color_t(255, 255, 255);
	Color_t colShadow = Color_t(56, 255, 125);

	Color_t colBackground = Color_t{15, 15, 15, 55};
	Color_t colOutline = Color_t{ 0, 0, 0, 55 };
};

#pragma endregion

/*
 * CONFIGURATION
 * - cheat variables serialization/de-serialization manager
 */
namespace C
{
	// member of user-defined custom serialization structure
	struct UserDataMember_t
	{
		// @todo: not sure is it possible and how todo this with projections, so currently done with pointer-to-member thing, probably could be optimized
		template <typename T, typename C>
		constexpr UserDataMember_t(const uint32_t uNameHash, const uint32_t uTypeHash, const T C::*pMember) :
			uNameHash(uNameHash), uTypeHash(uTypeHash), nDataSize(sizeof(std::remove_pointer_t<T>)), uBaseOffset(reinterpret_cast<std::size_t>(std::addressof(static_cast<C*>(nullptr)->*pMember))) { } // @test: 'CS_OFFSETOF' must expand to the same result but for some reason it doesn't

		// hash of custom variable name
		uint32_t uNameHash = 0U;
		// hash of custom variable type
		uint32_t uTypeHash = 0U;
		// data size of custom variable type
		std::size_t nDataSize = 0U;
		// offset to the custom variable from the base of class
		std::size_t uBaseOffset = 0U;
	};

	// user-defined custom serialization structure
	struct UserDataType_t
	{
		[[nodiscard]] std::size_t GetSerializationSize() const;

		uint32_t uTypeHash = 0U;
		std::vector<UserDataMember_t> vecMembers = {};
	};

	// variable info and value storage holder
	struct VariableObject_t
	{
		// @test: it's required value to be either trivially copyable or allocated/copied by new/placement-new operators, otherwise it may cause UB
		template <typename T> requires (!std::is_void_v<T> && std::is_trivially_copyable_v<T>)
		VariableObject_t(const uint32_t uNameHash, const uint32_t uTypeHash, const T& valueDefault) :
			uNameHash(uNameHash), uTypeHash(uTypeHash), nStorageSize(sizeof(T))
		{
#ifndef CS_NO_RTTI
			// store RTTI address if available
			this->pTypeInfo = &typeid(std::remove_cvref_t<T>);
#endif

			// @todo: do not call setstorage, instead construct it by placement-new operator
			// allocate storage on the heap if it doesnt't fit on the local one
			if constexpr (sizeof(T) > sizeof(this->storage.uLocal))
				this->storage.pHeap = HeapAlloc(GetProcessHeap(), 0, this->nStorageSize);

			SetStorage(&valueDefault);
		}

		VariableObject_t(VariableObject_t&& other) noexcept :
			uNameHash(other.uNameHash), uTypeHash(other.uTypeHash), nStorageSize(other.nStorageSize)
		{
#ifndef CS_NO_RTTI
			this->pTypeInfo = other.pTypeInfo;
#endif

			if (this->nStorageSize <= sizeof(this->storage.uLocal))
				memcpy(&this->storage.uLocal, &other.storage.uLocal, sizeof(this->storage.uLocal));
			else
			{
				this->storage.pHeap = other.storage.pHeap;

				// prevent it from being freed when the moved object is destroyed
				other.storage.pHeap = nullptr;
			}
		}

		VariableObject_t(const VariableObject_t& other) :
			uNameHash(other.uNameHash), uTypeHash(other.uTypeHash), nStorageSize(other.nStorageSize)
		{
#ifndef CS_NO_RTTI
			this->pTypeInfo = other.pTypeInfo;
#endif

			if (this->nStorageSize <= sizeof(this->storage.uLocal))
				memcpy(&this->storage.uLocal, &other.storage.uLocal, sizeof(this->storage.uLocal));
			else if (other.storage.pHeap != nullptr)
			{
				this->storage.pHeap = HeapAlloc(GetProcessHeap(), 0, this->nStorageSize);
				memcpy(this->storage.pHeap, other.storage.pHeap, this->nStorageSize);
			}
		}

		~VariableObject_t()
		{
			// check if heap memory is in use and allocated
			if (this->nStorageSize > sizeof(this->storage.uLocal) && this->storage.pHeap != nullptr)
				HeapFree(GetProcessHeap(), 0, this->storage.pHeap);
		}

		VariableObject_t& operator=(VariableObject_t&& other) noexcept
		{
			// check if heap memory is in use and allocated
			if (this->nStorageSize > sizeof(this->storage.uLocal) && this->storage.pHeap != nullptr)
				HeapFree(GetProcessHeap(), 0, this->storage.pHeap);

			this->uNameHash = other.uNameHash;
			this->uTypeHash = other.uTypeHash;
			this->nStorageSize = other.nStorageSize;

#ifndef CS_NO_RTTI
			this->pTypeInfo = other.pTypeInfo;
#endif

			if (this->nStorageSize <= sizeof(this->storage.uLocal))
				memcpy(&this->storage.uLocal, &other.storage.uLocal, sizeof(this->storage.uLocal));
			else
			{
				this->storage.pHeap = other.storage.pHeap;

				// prevent it from being freed when the moved object is destroyed
				other.storage.pHeap = nullptr;
			}

			return *this;
		}

		VariableObject_t& operator=(const VariableObject_t& other)
		{
			// check if heap memory is in use and allocated
			if (this->nStorageSize > sizeof(this->storage.uLocal) && this->storage.pHeap != nullptr)
				HeapFree(GetProcessHeap(), 0, this->storage.pHeap);

			this->uNameHash = other.uNameHash;
			this->uTypeHash = other.uTypeHash;
			this->nStorageSize = other.nStorageSize;

#ifndef CS_NO_RTTI
			this->pTypeInfo = other.pTypeInfo;
#endif

			if (this->nStorageSize <= sizeof(this->storage.uLocal))
				memcpy(&this->storage.uLocal, &other.storage.uLocal, sizeof(this->storage.uLocal));
			else if (other.storage.pHeap != nullptr)
			{
				this->storage.pHeap = HeapAlloc(GetProcessHeap(),0,this->nStorageSize);
				memcpy(this->storage.pHeap, other.storage.pHeap, this->nStorageSize);
			}

			return *this;
		}

		/// @tparam bTypeSafe if true, activates additional comparison of source and requested type information, requires RTTI
		/// @returns: pointer to the value storage, null if @a'bTypeSafe' is active and the access type does not match the variable type
		template <typename T, bool bTypeSafe = true> requires (std::is_object_v<T>)
		[[nodiscard]] const T* GetStorage() const
		{
			/*
#ifndef CS_NO_RTTI
			// sanity check of stored value type and asked value type
			if constexpr (bTypeSafe)
			{
				if (const std::type_info& currentTypeInfo = typeid(std::remove_cvref_t<T>); this->pTypeInfo != nullptr && CRT::StringCompare(this->pTypeInfo->raw_name(), currentTypeInfo.raw_name()) != 0)
				{
					if (char szPresentTypeName[64] = {}, szAccessTypeName[64] = {};
						MEM::fnUnDecorateSymbolName(this->pTypeInfo->raw_name() + 1U, szPresentTypeName, CS_ARRAYSIZE(szPresentTypeName), UNDNAME_NO_ARGUMENTS) != 0UL &&
						MEM::fnUnDecorateSymbolName(currentTypeInfo.raw_name() + 1U, szAccessTypeName, CS_ARRAYSIZE(szAccessTypeName), UNDNAME_NO_ARGUMENTS) != 0UL)
					{
						L_PRINT(LOG_ERROR) << CS_XOR("accessing variable of type: \"") << szPresentTypeName << CS_XOR("\" with wrong type: \"") << szAccessTypeName << CS_XOR("\"");
					}

					CS_ASSERT(false); // storage value and asked data type mismatch
					return nullptr;
				}
			}
#endif*/

			// check is value stored in the local storage
			if (this->nStorageSize <= sizeof(this->storage.uLocal))
				return reinterpret_cast<const std::remove_cvref_t<T>*>(&this->storage.uLocal);

			// otherwise it is allocated in the heap memory
			debug(this->storage.pHeap != nullptr); // tried to access non allocated storage
			return static_cast<const std::remove_cvref_t<T>*>(this->storage.pHeap);
		}

		template <typename T, bool bTypeSafe = true> requires (std::is_object_v<T>)
		[[nodiscard]] T* GetStorage()
		{
			return const_cast<T*>(static_cast<const VariableObject_t*>(this)->GetStorage<T, bTypeSafe>());
		}

		// replace variable contained value
		void SetStorage(const void* pValue);
		/// @returns: the size of the data to be serialized/de-serialized into/from the configuration file
		[[nodiscard]] std::size_t GetSerializationSize() const;

		// hash of variable name
		uint32_t uNameHash = 0x0;
		// hash of value type
		uint32_t uTypeHash = 0x0;
#ifndef CS_NO_RTTI
		// address of RTTI type data for value type
		const std::type_info* pTypeInfo = nullptr;
#endif
		// value storage size in bytes
		std::size_t nStorageSize = 0U;

		// value storage
		union
		{
			void* pHeap;
			std::uint8_t uLocal[sizeof(std::uintptr_t)]; // @test: expand local storage size to fit max possible size of trivial type so we can minimize heap allocations count
		} storage = { nullptr };
	};

	bool SaveSkinToJson(const std::size_t idx);

	bool LoadSkinFromJson(const std::size_t idx);

	bool LoadSkinFromJson(const std::size_t idx, EconItem_t& skin);

	// create directories and default configuration file
	bool Setup(const wchar_t* wszDefaultFileName);

	/* @section: main */
	// loop through directory content and store all user configurations filenames
	void Refresh();
	/// register user-defined data structure type and it's member variables
	/// @param[in] vecUserMembers member variables of structure that needs to be serialized/de-serialized
	void AddUserType(const uint32_t uTypeHash, std::initializer_list<UserDataMember_t> vecUserMembers);
	void DeserializeEconItem(const VariableObject_t& variable, const EconItem_t& item);
	void  SerializeEconItem(VariableObject_t& variable, const EconItem_t& item);
	bool SaveEconItem(const std::size_t nFileIndex, const EconItem_t& item);
	bool LoadEconItem(const std::size_t nFileIndex, const EconItem_t& item);

	/// write/re-write single variable to existing configuration file
	/// @returns: true if variable has been found or created and successfully written, false otherwise
	bool SaveFileVariable(const std::size_t nFileIndex, const VariableObject_t& variable);
	/// read single variable from existing configuration file
	/// @remarks: when the version of cheat is greater than version of the configuration file and @a'variable' wasn't found, this function saves it and updates the version to the current one, note that it doesn't affect to return value
	/// @returns: true if variable has been found and successfully read, false otherwise
	bool LoadFileVariable(const std::size_t nFileIndex, VariableObject_t& variable);
	/// erase single variable from existing configuration file
	/// @returns: true if variable did not exist or was successfully removed, false otherwise
	bool RemoveFileVariable(const std::size_t nFileIndex, const VariableObject_t& variable);
	/// create a new configuration file and save it
	/// @param[in] wszFileName file name of configuration file to save and write in
	/// @returns: true if file has been successfully created and all variables were written to it, false otherwise
	bool CreateFile(const wchar_t* wszFileName);
	/// serialize variables into the configuration file
	/// @param[in] nFileIndex index of the exist configuration file name
	/// @returns: true if all variables were successfully written to the file, false otherwise
	bool SaveFile(const std::size_t nFileIndex);
	/// de-serialize variables from the configuration file
	/// @param[in] nFileIndex index of the exist configuration file name
	/// @returns: true if all variables were successfully loaded from the file, false otherwise
	bool LoadFile(const std::size_t nFileIndex);
	/// remove configuration file
	/// @param[in] nFileIndex index of the exist configuration file name
	void RemoveFile(const std::size_t nFileIndex);

	/* @section: values */
	// all user configuration filenames
	inline std::vector<wchar_t*> vecFileNames = {};
	// custom user-defined serialization data types
	inline std::vector<UserDataType_t> vecUserTypes = {};
	// configuration variables storage
	inline std::vector<VariableObject_t> vecVariables = {};

	/* @section: get */
	/// @returns: index of variable with given name hash if it exist, 'C_INVALID_VARIABLE' otherwise
	[[nodiscard]] std::size_t GetVariableIndex(const uint32_t uNameHash);

	/// @tparam T type of variable we're going to get, must be exactly the same as when registered
	/// @returns: variable value at given index
	template <typename T>
	[[nodiscard]] T& Get(const std::size_t nIndex)
	{
		return *vecVariables[nIndex].GetStorage<T>();
	}
	
	// @todo: get rid of templates, so it doesn't compile duplicates and we're able to merge things to .cpp
	/// add new configuration variable
	/// @returns: index of added variable
	template <typename T> requires (!std::is_array_v<T>)
	std::size_t AddVariable(const uint32_t uNameHash, const uint32_t uTypeHash, const T& valueDefault)
	{
		vecVariables.emplace_back(uNameHash, uTypeHash, valueDefault);
		return vecVariables.size() - 1U;
	}
	template <typename T>
	bool Set(const std::size_t nIndex, const T& value)
	{
		if (nIndex < vecVariables.size())
		{
			vecVariables[nIndex].SetStorage(&value);
			return true;
		}

		return false;
	}
	/// add new configuration array variable initialized by single value
	/// @returns: index of added array variable
	template <typename T> requires (std::is_array_v<T>)
	std::size_t AddVariableArray(const uint32_t uNameHash, const uint32_t uTypeHash, const std::remove_pointer_t<std::decay_t<T>> valueDefault)
	{
		using BaseType_t = std::remove_pointer_t<std::decay_t<T>>;

		T arrValueDefault;
		for (std::size_t i = 0U; i < sizeof(T) / sizeof(BaseType_t); i++)
			arrValueDefault[i] = valueDefault;

		vecVariables.emplace_back(uNameHash, uTypeHash, arrValueDefault);
		return vecVariables.size() - 1U;
	}

	/// add new configuration array variable with multiple values initialized
	/// @returns: index of added array variable
	template <typename T> requires (std::is_array_v<T>)
	std::size_t AddVariableArray(const uint32_t uNameHash, const uint32_t uTypeHash, std::initializer_list<std::remove_pointer_t<std::decay_t<T>>> vecValuesDefault)
	{
		using BaseType_t = std::remove_pointer_t<std::decay_t<T>>;

		T arrValueDefault;
		memset(arrValueDefault, 0U, sizeof(T));
		memcpy(arrValueDefault, vecValuesDefault.begin(), vecValuesDefault.size() * sizeof(BaseType_t));

		vecVariables.emplace_back(uNameHash, uTypeHash, arrValueDefault);
		return vecVariables.size() - 1U;
	}

	inline void RemoveVariable(const std::size_t nIndex)
	{
		vecVariables.erase(vecVariables.begin() + nIndex);
	}
}
using VisualOverlayBox_t = int;

enum EVisualOverlayBox : VisualOverlayBox_t
{
	VISUAL_OVERLAY_BOX_NONE = 0,
	VISUAL_OVERLAY_BOX_FULL,
	VISUAL_OVERLAY_BOX_CORNERS,
	VISUAL_OVERLAY_BOX_MAX
};

using ESPFlags_t = unsigned int;
enum EESPFlags : ESPFlags_t
{
	FLAGS_NONE = 0U,
	FLAGS_ARMOR = 1 << 0,
	FLAGS_DEFUSER = 1 << 1
};
enum e_chams_type : int {
	enemy = 0,
	local = 1,
	team = 2,
	viewmodel = 3,
	wep = 4,

};
struct vars_t
{
	/* rage */
	add_cfg(int, text_type, 0);

	add_adaptive_cfg(int, 7, rage_target_select, 0);
	add_adaptive_cfg(int, 7, rage_minimum_damage, 0);
	add_adaptive_cfg(int, 7, rage_minimum_damage_override, 0);

	add_adaptive_cfg(int, 7, rage_minimum_hitchance, 0);
	add_adaptive_cfg(int, 7, rage_hitboxes, 0);

	add_cfg(float, max_time_ms, 1000);
	add_cfg(int, rage_weapon_selection, 1);

	add_cfg(bool, safe_mode, false);
	add_cfg(bool, at_targets, false);	
	add_cfg(bool, anti_aim, false);


	add_cfg(bool, rage_enable, false);
	add_adaptive_cfg(bool, 7, rage_hitchance, 0);
	add_adaptive_cfg(bool, 7, rapid_fire, 0);
	add_adaptive_cfg(bool, 7, rage_penetration, 0);
	add_adaptive_cfg(bool, 7, rage_safe_point, 0);
	add_adaptive_cfg(bool, 7, rage_auto_stop, 0);
	add_adaptive_cfg(bool, 7, rage_early_stop, 0);
	add_adaptive_cfg(bool, 7, rage_auto_scope, 0);
	add_cfg(bool,  remove_weapon_accuracy_spread, false);
	add_adaptive_cfg(int, 7, rage_silent_mode, 0);
	add_adaptive_cfg(bool, 7, hitbox_head, 0);
	add_adaptive_cfg(bool, 7, prefer_body, 0);

	add_adaptive_cfg(bool, 7, hitbox_neck, 0);
	add_adaptive_cfg(bool, 7, hitbox_uppeer_chest, 0);
	add_adaptive_cfg(bool, 7, hitbox_chest, 0);
	add_adaptive_cfg(bool, 7, hitbox_stomach, 0);
	add_adaptive_cfg(bool, 7, hitbox_legs, 0);
	add_adaptive_cfg(bool, 7, hitbox_feet, 0);
	/* rage end*/
	add_cfg(int, rage_fov, 0);

	std::vector<EconItem_t> vecEconItems;

	add_cfg(bool, hit_sound, false);

	/* legit begin*/
	add_cfg(int, legit_weapon_selection, 1);

	add_cfg(bool, legit_enable, false);
	add_cfg(bool, legit_trigger_bot_on_key, false);
	add_cfg(int, legit_target_delay, 0);

	add_cfg(bool, key_bind_list, false);
	add_cfg(bool, spectator_list, false);
	add_cfg(int, max_ticks, 0);
	add_cfg(KeyBind_t, legit_trigger_bot_key, 0);
	add_cfg(KeyBind_t, min_dmg_key, 0);
	add_cfg(int, legit_key_style, 0);
	add_cfg(int, legit_trigger_bot_key_style, 0);
	add_adaptive_cfg(bool, 7, trigger_on_key, false);
	add_adaptive_cfg(int, 7,trigger_shots, 0);
	add_adaptive_cfg(int, 7,trigger_delay, 0);
	add_adaptive_cfg(int,7, trigger_dmg, 0);
	add_adaptive_cfg(unsigned int, 7, trigger_hitbox, 0);
	add_adaptive_cfg(float, 7, trigger_hitchance, 0);
	
	add_adaptive_cfg(bool, 7, legit_rcs_smoothx_pistol, 0);
	add_adaptive_cfg(float, 7, legit_smooth_x, 1.f);
	add_adaptive_cfg(float, 7, legit_smooth_y, 1.f);

	add_adaptive_cfg(bool, 7, legit_curve, 0);
	add_adaptive_cfg(float, 7, legit_curve_x, 0.f);
	add_adaptive_cfg(float, 7, legit_curve_y, 0.f);


	add_adaptive_cfg(bool, 7, legit_smoothing, 0);
	add_adaptive_cfg(bool, 7, legit_triggerbot, false);
	add_adaptive_cfg(int, 7, legit_target_select, 0);
	add_adaptive_cfg(float, 7, legit_fov, 90);
	add_adaptive_cfg(bool, 7, legit_rcs, 0);
	add_adaptive_cfg(int, 7, legit_head_scale, 0);
	add_adaptive_cfg(int, 7, legit_body_scale, 0);
	add_cfg(bool, render_multipoints, false);

	add_adaptive_cfg(float, 7, legit_rcs_x, 0);
	add_adaptive_cfg(float, 7, legit_rcs_y, 0);
	add_adaptive_cfg(bool, 7, legit_rcs_humanization, false);
	add_adaptive_cfg(bool, 7, legit_silent, false);
	add_adaptive_cfg(bool, 7, perfect_silent, false);
	add_adaptive_cfg(bool, 7, recoil_adjust, false);
	add_adaptive_cfg(int, 7, recoil_shots, 0);
	add_adaptive_cfg(int, 7, rcsy, 0);
	add_adaptive_cfg(int, 7, rcsx, 0);
	add_adaptive_cfg(int, 7, rcs_human, 0);
	add_adaptive_cfg(int, 7, rcshuman, 0);
	add_adaptive_cfg(unsigned int, 7, legit_hitboxes, 0);
	add_adaptive_cfg(bool, 7, trigger_magnet, false);
	add_adaptive_cfg(bool, 7, trigger_rcs, 0);
	add_cfg(KeyBind_t, legit_key0, 0);
	add_cfg(KeyBind_t, legit_key1, 0);
	add_cfg(KeyBind_t, legit_key2, 0);
	add_cfg(KeyBind_t, legit_key3, 0);
	add_cfg(KeyBind_t, legit_key4, 0);
	add_cfg(KeyBind_t, legit_key5, 0);
	add_cfg(KeyBind_t, legit_key6, 0);
	add_cfg(KeyBind_t, legit_key7, 0);

	add_adaptive_cfg(float, 7, legit_smooth_accel, 0);

	add_cfg(int, legit_history, 0);
	add_cfg(int, legit_fov_type, 0);
	add_cfg(bool, renderize_fov, false);

	add_cfg(bool, renderize_rage_fov, false);
	add_cfg(ColorPickerVar_t, renderize_fov_rage_clr, ColorPickerVar_t(0, 255, 0, 55));
	add_cfg(ColorPickerVar_t, renderize_fov_clr_hit, ColorPickerVar_t(11, 155, 0, 55));
	add_cfg(ColorPickerVar_t, renderize_fov_clr, ColorPickerVar_t(155, 11, 0, 55));
	add_cfg(ColorPickerVar_t, smoke_color, ColorPickerVar_t(0, 255, 0, 55));
	add_cfg(bool, kill_effects, false);

	add_cfg(bool, night_mode, false);
	add_cfg(ColorPickerVar_t, night_mode_col, ColorPickerVar_t(15, 15, 18, 155));
	add_cfg(bool, sun, false);
	add_cfg(ColorPickerVar_t, sun_col, ColorPickerVar_t(155, 55, 55, 55));
	add_cfg(bool, props, false);
	add_cfg(ColorPickerVar_t, props_col, ColorPickerVar_t(15, 15, 18, 155));

	add_cfg(bool, lighting, false);
	add_cfg(ColorPickerVar_t, lighting_col, ColorPickerVar_t(15, 15, 18, 155));
	add_cfg(bool, rage_auto_fire, false);
	add_cfg(int, aa_pitch, 0);
	add_cfg(int, aa_yaw, 0);
	add_cfg(bool, movement_quick_stop, 0);
	add_cfg(bool, custom_smoke_color, false);
	add_cfg(bool, view_model, false);
	add_cfg(float, view_model_y, 0);
	add_cfg(float, view_model_x, 0);
	add_cfg(float, view_model_z, 0);
	add_cfg(bool, camera_fov, false);
	add_cfg(float, camera_fov_value, 0);
	add_cfg(float, camera_fov_value_scoped, 0);
	add_cfg(bool, remove_scope, false);
	add_adaptive_cfg(int, 7, point_scale, 0);
	add_adaptive_cfg(int, 7, head_scale, 0);


	add_cfg(bool, hand_switch, false);
	add_cfg(bool, aa_manual_right, false);
	add_cfg(bool, aa_manual_left, false);

	add_cfg(KeyBind_t, third_person_key, 0);
	add_cfg(KeyBind_t, hand_switch_key, 0);
	add_cfg(KeyBind_t, right_key, 0);
	add_cfg(KeyBind_t, left_key, 0);

	add_cfg(int, hand_switch_key_style, 0);
	add_cfg(float, view_model_fov, 90);
	/* legit end */
	add_cfg(bool, thirdperson, true);
	add_cfg(int, thirdperson_fov, 90);
	add_cfg(int, hitmarker_dist, 0);
	add_cfg(float, hitmarker_expire, 0.f);
	add_cfg(int, hitmarker_size, 0);
	add_cfg(bool, dmg_indicator, false);
	add_cfg(bool, bunny_hop, true);
	add_cfg(bool, bSkeleton, false);
	add_cfg(bool, hitmarker, false);
	add_cfg(bool, hitmarker3d, false);

	add_cfg(bool, velocity_indicator, false);
	add_cfg(bool, bSkeletonLagComp, false);
	add_cfg(bool, bVisualChamsRagDoll, false);
	add_cfg(bool,  bVisualChams, false);
	add_cfg(int,  nVisualChamMaterial, 0);
	add_cfg(int, nVisualChamMaterial2, 0);
	add_cfg(bool, grenade_warning_team, false);
	add_cfg(bool, grenade_warning, false);

	add_cfg(bool, bVisualChamsIgnoreZ, false); // invisible chams
	add_cfg(ColorPickerVar_t,  colVisualChams, ColorPickerVar_t(0, 255, 0));
	add_cfg(ColorPickerVar_t,  colVisualChamsIgnoreZ, ColorPickerVar_t(255, 0, 0));
	add_cfg(bool, unlock_inventory, false);
	add_cfg(bool, grenade_prediction, false);
	add_cfg(bool, grenade_prediction_radius, false);
	add_cfg(bool, grenade_projectile, false);

	add_cfg(ColorPickerVar_t, grenade_prediction_col, ColorPickerVar_t(0, 0, 155, 55));
	add_cfg(ColorPickerVar_t, grenade_prediction_radius_col, ColorPickerVar_t(0, 0, 155, 55));

	add_cfg(bool, off_screen, false);
	add_cfg(ColorPickerVar_t, off_screen_clr, ColorPickerVar_t(0, 0, 155, 55));

	add_cfg(bool, bVisualChamsL, false);
	add_cfg(int, nVisualChamMaterialL, 0);
	add_cfg(int, nVisualChamMaterialT2, 0);

	add_cfg(bool, bVisualChamsIgnoreZL, false); // invisible chams
	add_cfg(ColorPickerVar_t, colVisualChamsL, ColorPickerVar_t(0, 255, 0));
	add_cfg(ColorPickerVar_t, colVisualChamsIgnoreZL, ColorPickerVar_t(255, 0, 0));

	add_cfg(bool, bullet_beam_enemy, false);
	add_cfg(ColorPickerVar_t, bullet_beam_enemy_clr, ColorPickerVar_t(0, 255, 0));
	add_cfg(bool, bullet_beam_local, false);
	add_cfg(float, bullet_beam_width, 1.5);
	add_cfg(ColorPickerVar_t, bullet_beam_local_clr, ColorPickerVar_t(0, 255, 0));
	add_cfg(ColorPickerVar_t, hitmarker_col, ColorPickerVar_t(0, 255, 0));

	add_cfg(bool, bVisualChamsT, false);
	add_cfg(int, nVisualChamMaterialT, 0);
	add_cfg(bool, bVisualChamsIgnoreZT, false); // invisible chams
	add_cfg(ColorPickerVar_t, colVisualChamsT, ColorPickerVar_t(0, 255, 0));
	add_cfg(ColorPickerVar_t, colVisualChamsIgnoreZT, ColorPickerVar_t(255, 0, 0));


	add_cfg(bool, bVisualChamsV, false);
	add_cfg(int, nVisualChamMaterialV, 0);
	add_cfg(bool, bVisualChamsIgnoreZV, false); // invisible chams
	add_cfg(ColorPickerVar_t, colVisualChamsV, ColorPickerVar_t(0, 255, 0));
	add_cfg(ColorPickerVar_t, colVisualChamsIgnoreZV, ColorPickerVar_t(255, 0, 0));


	add_cfg(bool, bVisualChamsW, false);
	add_cfg(int, nVisualChamMaterialW, 0);
	add_cfg(bool, bVisualChamsIgnoreZW, false); // invisible chams
	add_cfg(ColorPickerVar_t, colVisualChamsW, ColorPickerVar_t(0, 255, 0));
	add_cfg(ColorPickerVar_t, colVisualChamsIgnoreZW, ColorPickerVar_t(255, 0, 0));
	add_cfg(bool, bGlow, false);
	add_cfg(ColorPickerVar_t, bGlowClr, ColorPickerVar_t(255, 0, 0));

	add_cfg(bool, molotov_radius, false);
	add_cfg(ColorPickerVar_t, molotov_radius_clr, ColorPickerVar_t(255, 0, 0));

	add_cfg(bool, bNoShadow, false);
	add_cfg(bool, bVisualOverlay, true);

	add_cfg(bool, bRemoveChamsT, false);

	add_cfg(bool, bRemoveChamsOcclude, false);

	add_cfg(ColorPickerVar_t, colSkeleton, ColorPickerVar_t(88, 88, 88)); // (text)
	add_cfg(ColorPickerVar_t, colSkeletonOutline, ColorPickerVar_t(0, 0, 0)); // (text)

	add_cfg(ColorPickerVar_t, colModulate, ColorPickerVar_t(255, 0, 0));
	add_cfg(ColorPickerVar_t, colSkeletonBacktrack, ColorPickerVar_t(255, 0, 0));
	add_cfg(unsigned int, pEspFlags, FLAGS_NONE);
	add_cfg(TextOverlayVar_t, HKFlag, TextOverlayVar_t(true, false));
	add_cfg(TextOverlayVar_t, KitFlag, TextOverlayVar_t(true, false));

	add_cfg(TextOverlayVar_t, Weaponesp, TextOverlayVar_t(false, true));

	add_cfg(FrameOverlayVar_t, overlayBox, FrameOverlayVar_t(false));
	add_cfg(TextOverlayVar_t, overlayName, TextOverlayVar_t(true, false));
	add_cfg(BarOverlayVar_t, overlayHealthBar, BarOverlayVar_t(false, false, false, 2.1f, Color_t(0, 255, 155), Color_t(255, 0, 155)));
	add_cfg(BarOverlayVar_t, backgroundHealthbar, BarOverlayVar_t(false, false, false, 1.f, Color_t(0, 0, 0, 155), Color_t(0, 0, 0, 155)));

	add_cfg(BarOverlayVar_t, AmmoBar, BarOverlayVar_t(false, false, false, 2.1f, Color_t(0, 255, 155), Color_t(255, 0, 155)));
	add_cfg(BarOverlayVar_t, AmmoBarBackground, BarOverlayVar_t(false, false, false, 2.1f, Color_t(0, 0, 0, 155), Color_t(0, 0, 0, 155)));
	add_cfg(unsigned int, legit_ignore, 0);
	add_cfg(unsigned int, legit_hitbox, 0);

	/* world */

	add_cfg(int, entity_type, 0);
	add_cfg(bool, weapon_esp, false);
	add_cfg(FrameOverlayVar_t, weapon_bbox, FrameOverlayVar_t(false));
	add_cfg(TextOverlayVar_t, weapon_name, TextOverlayVar_t(false, true));
	add_cfg(BarOverlayVar_t, weapon_ammo_bar, BarOverlayVar_t(false, false, false, 1.8, Color_t(0, 255, 155), Color_t(255, 0, 155)));
	add_cfg(BarOverlayVar_t, weapon_ammo_bar_bg, BarOverlayVar_t(false, false, false, 1.f, Color_t(0, 0, 0, 155), Color_t(0, 0, 0, 155)));


	add_cfg(FrameOverlayVar_t, projectile_box, FrameOverlayVar_t(false));
	add_cfg(TextOverlayVar_t, projectile_name, TextOverlayVar_t(false, true));
	add_cfg(int, projectile_name_type, 0);
	add_cfg(bool, projectile_esp, false);

	add_cfg(bool, motion_blur, false);
	add_cfg(float, m_flMaxExposure, 0.f);
	add_cfg(float, m_flMinExposure, 0.f);
	add_cfg(float, m_flFadeDuration, 0.f);
	add_cfg(bool, control_exposure, false);
	add_cfg(bool, compensate_exposure, false);
	add_cfg(float, m_flExposureCompensation, 0.f);
	add_cfg(float, m_flExposureFadeSpeedUp, 0.f);
	add_cfg(float, m_flExposureFadeSpeedDown, 0.f);
	add_cfg(bool, modulate_sky, false);
	add_cfg(float, sky_brightness, 0.f);
	add_cfg(ColorPickerVar_t, sky_col, ColorPickerVar_t(16, 15, 18));
	add_cfg(ColorPickerVar_t, accent_col, ColorPickerVar_t(16, 15, 222));
	add_cfg(unsigned int, game_removals, 0);

	add_cfg(bool, auto_strafe, false);
	add_cfg(bool, auto_strafe_p, false);
	add_cfg(bool, semirage, false);
	add_cfg(int, semiragehc, 0.f);
	add_cfg(float, mindmg, 0.f);
	add_cfg(bool, between_shots, false);
	add_cfg(float, aspect_ratio_value, 1.3f);
	add_cfg(bool, aspect_ratio, false);

	add_cfg(bool, auto_strafe_wasd, false);
	add_cfg(float, auto_strafe_smooth, 0.f);
	
		add_cfg(int, chams_type, 0);
	add_cfg(KeyBind_t, edge_jump_key, 0);
	add_cfg(KeyBind_t, long_jump_key, 0);
	add_cfg(KeyBind_t, ladder_jump_key, 0);
	add_cfg(KeyBind_t, mini_jump_key, 0);
	add_cfg(KeyBind_t, edge_bug_key, 0);
	add_cfg(bool, fast_ladder, false);
	add_cfg(int, long_jump_key_style, 0);
	add_cfg(int, edge_jump_style, 0);
	add_cfg(int, ladder_style, 0);
	add_cfg(int, mini_style, 0);
	add_cfg(int, edge_bug_style, 0);

	add_cfg(bool, impacts, false);
	add_cfg(bool, edge_bug_type, 0);
	add_cfg(bool, edge_jump_on_key, false);	
	add_cfg(bool, edge_jump, false);
	add_cfg(bool, mini_jump, false);
	add_cfg(bool, mini_jump_on_key, false);
	add_cfg(bool, ladder_jump, false);
	add_cfg(bool, ladder_jump_on_key, false);
	add_cfg(bool, long_jump, false);
	add_cfg(bool, long_jump_on_key, false);
	add_cfg(bool, edge_bug, false);
	add_cfg(bool, edge_bug_on_key, false);
	add_cfg(bool, scope_line, false);
	add_cfg(float, scope_line_size, 0.f);
	add_cfg(ColorPickerVar_t, scope_line_color, ColorPickerVar_t(16, 15, 18, 155));
	add_cfg(bool, grenade_warning_flash, false);
	add_cfg(bool, grenade_warning_smoke, false);
	add_cfg(bool, grenade_warning_molotov, false);
	add_cfg(bool, clouds, false);
	add_cfg(ColorPickerVar_t, cloud_clr, ColorPickerVar_t(222, 155, 18, 255));
	add_cfg(bool, effects, false);
	add_cfg(ColorPickerVar_t, effects_clr, ColorPickerVar_t(55, 55, 55, 255));
	add_cfg(bool, decals, false);
	add_cfg(ColorPickerVar_t, decal_clr, ColorPickerVar_t(55, 55, 55, 255));

	add_cfg(bool, out_of_view, false);
	add_cfg(ColorPickerVar_t, out_of_view_clr, ColorPickerVar_t(222, 15, 18, 222));
	add_cfg(int, out_of_view_size, 0);
	add_cfg(int, out_of_view_distance, 0);
	add_cfg(bool, obs_proof, false);


	add_cfg(int, size1, 0);
	add_cfg(int, size2, 0);
	add_cfg(int, size3, 0);
	add_cfg(int, size4, 0);
	add_cfg(int, size5, 0);

	add_cfg(ColorPickerVar_t, color1, ColorPickerVar_t(222, 155, 18, 255));
	add_cfg(ColorPickerVar_t, color12, ColorPickerVar_t(222, 155, 18, 255));
	add_cfg(ColorPickerVar_t, color13, ColorPickerVar_t(222, 155, 18, 255));
	add_cfg(ColorPickerVar_t, color14, ColorPickerVar_t(222, 155, 18, 255));
	add_cfg(ColorPickerVar_t, color15, ColorPickerVar_t(222, 155, 18, 255));
	add_cfg(ColorPickerVar_t, color16, ColorPickerVar_t(222, 155, 18, 255));
	add_cfg(ColorPickerVar_t, color17, ColorPickerVar_t(222, 155, 18, 255));

};

inline vars_t g_cfg = {};
