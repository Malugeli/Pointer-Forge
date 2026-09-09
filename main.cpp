#include <iostream>
#define NOMINMAX
#include <Windows.h>
#include <TlHelp32.h>
#include <optional>
#include <cstdint>
#include <utility>
#include <string>
#include <string_view>
#include <limits>
#include <unordered_map>
#include <array>
#include <chrono>
#include <thread>
#include <ranges>


struct unique_handle {

private:
	HANDLE h = nullptr;

public:
	//K onstruktoren:
	constexpr unique_handle() = default; //Ohne Argument geben wir leeres unique_handle zurück
	explicit unique_handle(HANDLE handle) noexcept
		: h(handle == INVALID_HANDLE_VALUE ? nullptr : handle) { //Initialisierungsliste mit Ternary Operator
	}; //mit Argument "Wrappen" wir das Handle (wir öffnen OpenProcess hier drin)

	//Destruktor:
	~unique_handle() noexcept { reset(); }; //Die Resetfunction ist weiter unten

	//Kopieren verbieten:
	unique_handle(const unique_handle&) = delete; // für unique_handle b(bestehendes unique_handle); löschen wir die Funktion
	unique_handle& operator=(const unique_handle&) = delete; //für bestehende unique_handle a = b; löschen wir die Funktion

	//Verschieben erlauben:
	unique_handle(unique_handle&& other) noexcept
		: h(std::exchange(other.h, nullptr)) {
	} //Das ist für ein neues Handle unique_handle a(std:move(b)) oder unique_handle a = std::move(b).
	unique_handle& operator=(unique_handle&& other) noexcept { //das ist für ein bestehendes Handle a = std::move(b)
		if (this != &other) {
			reset();
			h = std::exchange(other.h, nullptr);
		}
		return *this;
	}

	void reset(HANDLE nh = nullptr) { // Falls wir kein Parameter geben und nur a.reset() machen dann wird das Handle darin geschlossen. Machen wir aber a.reset(OpenProcess..) wird das neue Handle eingefügt.
		if (h && h != INVALID_HANDLE_VALUE) {
			CloseHandle(h);
		}
		h = (nh == INVALID_HANDLE_VALUE ? nullptr : nh);
	}

	HANDLE release() { //gibt das Handle ab. Das unique_handle gehört uns nicht mehr.
		return std::exchange(h, nullptr);
	}

	friend void swap(unique_handle& a, unique_handle& b) noexcept {
		std::swap(a.h, b.h);
	}

	//Observerfunktionen
	[[nodiscard]] HANDLE get() const noexcept {
		return h;
	}

	[[nodiscard]] explicit operator bool() const noexcept {
		return h && h != INVALID_HANDLE_VALUE;
	}
};

struct CrusaderStruct {
	unique_handle process;
	std::uintptr_t baseadresse;
};

struct material {
	std::string_view mats;
	uint32_t offset;
};

constexpr std::array maher{
	material{"Gold", 0xD5FCF8},
	material{"Holz", 0xB9AC6C},
	material{"Eisen", 0xB9AFA8},
	material{"Pech", 0xB9B2D8},
	material{"Beliebtheit", 0xD5F84C}
	};

[[nodiscard]] std::optional<CrusaderStruct>
FindGame(const wchar_t*, const wchar_t*) noexcept;

std::optional<std::uintptr_t>
GetModuleBaseAdresse(std::wstring_view moduleName, DWORD pID) noexcept;

uint8_t get_wish();

bool cheat(HANDLE h, uintptr_t baseadresse, uint8_t index);

int main()
{
	SetConsoleOutputCP(CP_UTF8);
	auto crusader = FindGame(L"Crusader", L"Stronghold Crusader.exe");
	if (!crusader) {
		std::cout << "Game not found" << '\n';
		return 0;
	}
	while(true)
	{
		if (!cheat(crusader->process.get(), crusader->baseadresse, get_wish())) {
			std::this_thread::sleep_for(std::chrono::seconds(5));
			return 0;
		}
	}


}


[[nodiscard]] std::optional<std::uintptr_t>
GetModuleBaseAdresse(std::wstring_view moduleName, DWORD pID) noexcept { //Wir nutzen wchar_t weil wir unten mit einem wchar_t[] vergleichen.
	unique_handle h(CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, pID)); //Funktion returned ein Handle. Wir wrappen es.
	MODULEENTRY32 GameModule{};
	GameModule.dwSize = sizeof(MODULEENTRY32);

	if (h && Module32First(h.get(), &GameModule))
	{
		do {
			if (moduleName == GameModule.szModule) 
			{
				return reinterpret_cast<std::uintptr_t>(GameModule.modBaseAddr);
			}
		} while (Module32Next(h.get(), &GameModule));
	}

	return std::nullopt;

};

[[nodiscard]] std::optional<CrusaderStruct>
FindGame(const wchar_t* window_name, const wchar_t* module_name) noexcept {
	HWND GameWindowHandle = FindWindow(NULL, window_name);
	if (!GameWindowHandle) {
		return std::nullopt;
	}

	DWORD pID{};
	GetWindowThreadProcessId(GameWindowHandle, &pID);
	if (!pID) {
		return std::nullopt;
	}

	unique_handle handle(OpenProcess(PROCESS_ALL_ACCESS, FALSE, pID));
	if (!handle) {
		return std::nullopt;
	}

	auto baseAdresse = GetModuleBaseAdresse(module_name, pID);
	if (!baseAdresse) {
		return std::nullopt;
	}

	return CrusaderStruct{ std::move(handle), baseAdresse.value() };
}


uint8_t get_wish() {

	while (true) {
	std::string input{};;
	std::cout << "Was möchtest du haben?" << '\n';
	for (auto [i, mat] : (std::views::enumerate(maher))) {
		std::cout << "Tippe " << i + 1 << " für " << mat.mats << '\n';
	}
	std::getline(std::cin, input);
		try {
			unsigned long temp = std::stoul(input);
			if (temp > maher.size()) {
				throw std::out_of_range("Bitte gültige Zahl angeben");
			}
			return static_cast<uint8_t>(temp - 1);
		}
		catch (const std::out_of_range& e) {
			std::cout << e.what() << '\n';
		}
		catch (const std::invalid_argument) {
			std::cout << "Bitte nur die Zahl angeben" << '\n';
		}
	}

}

bool cheat(HANDLE h, uintptr_t baseadresse, uint8_t index) {
	std::uint32_t stand{};
	std::string wunsch{};
	void* adresse = reinterpret_cast<void*>(baseadresse + maher[index].offset);
	if (!ReadProcessMemory(h, adresse, &stand, sizeof(stand), 0)){
		std::cout << "Fehler beim Lesen der Memory, Programm wird gschlossen" << '\n';
		return false;
	}
	std::cout << "Aktueller " << maher[index].mats << "stand: " << stand << '\n';
	std::cout << "Wie viel " << maher[index].mats << " willst du haben? " << '\n';
	std::getline(std::cin, wunsch);
	try {
		unsigned long temp = std::stoul(wunsch);
		if (temp > std::numeric_limits<uint32_t>::max()){
			throw std::out_of_range("Übertreib nicht...");
		}
		stand += temp;
	}
	catch (const std::out_of_range& e) {
		std::cout << e.what() << '\n';
	}
	catch (const std::invalid_argument) {
		std::cout << "Bitte nur Zahlen angeben" << '\n';
	}
	if(!WriteProcessMemory(h, adresse, &stand, sizeof(stand), 0))
	{
		std::cout << "Fehler beim Schreiben der Memory, Programm wird geschlossen" << '\n';
		return false;
	}
	return true;
}