// 2026-01-14T20:55
#ifndef CAM_COMMANDS_H
#define CAM_COMMANDS_H

#include <QString>
#include <array>

// ---------------------
// X-macro list (single source of truth)
#define CAM_COMMANDS_LIST \
	CAM_COMMANDS_HELPER(URL_H) \
	CAM_COMMANDS_HELPER(CMD_NIGHT_ON) \
	CAM_COMMANDS_HELPER(CMD_NIGHT_OFF)

class CamCommands {
public:
	// enum
	enum class Command {
#define CAM_COMMANDS_HELPER(name) name,
		CAM_COMMANDS_LIST
#undef CAM_COMMANDS_HELPER
		Count
	};

	// Header-only static array
	inline static const std::array<QString, static_cast<int>(Command::Count)> CommandStrings = []{
#define CAM_COMMANDS_HELPER(name) QStringLiteral(#name),
		return std::array<QString, static_cast<int>(Command::Count)>{ CAM_COMMANDS_LIST };
#undef CAM_COMMANDS_HELPER
	}();

	// Helper function
	static QString toString(Command cmd) {
		int idx = static_cast<int>(cmd);
		if (idx < 0 || idx >= static_cast<int>(Command::Count))
			return QStringLiteral("Unknown");
		return CommandStrings[idx];
	}
};

// Cleanup macro
#undef CAM_COMMANDS_LIST

#endif // CAM_COMMANDS_H

#if 0
enum class Type : int { A, B, C, Count };
using U = std::underlying_type_t<Type>;
U n = static_cast<U>(Type::Count);
for (U i = 0; i < static_cast<U>(Type::Count); ++i) {
    Type t = static_cast<Type>(i);
    // ...
    }
endif #