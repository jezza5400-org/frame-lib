# frame-lib

AX.25 and KISS framing libraries.

## Note

Log a vector:

```cpp
#include "spdlog/spdlog.h"
#include "spdlog/fmt/ranges.h"

spdlog::info("Bytes (0xHEX): [{:#04X}]", spdlog::fmt_lib::join(frame, ", "));
```
