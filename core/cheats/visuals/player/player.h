#pragma once

#include <memory>
#include <array>

#include "..\..\..\utils\render\render.h"

#include "..\..\..\utils\utils.h"
#include "..\..\..\sdk\sdk.h"
#include "..\..\..\math\elements\qangle.h"
#include "..\..\..\math\elements\vector.h"
#include "..\..\..\math\elements\transform.h"
#include "..\..\..\utils\configs\config.h"

namespace player
{
	class c_esp {
	public:
		void run();

	};
	const auto esp = std::make_unique<c_esp>();
}