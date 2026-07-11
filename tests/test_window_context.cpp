#include <GLFW/glfw3.h>

#include <catch2/catch_test_macros.hpp>

#include "WindowContext.h"

TEST_CASE("WindowContext Initialization and Lifecycle", "[WindowContext]") {
  WindowContext ctx;

  SECTION("Initialization and window creation") {
    // Attempt to initialize the window context.
    // In our test environment (with X11), glfwInit() and window creation should
    // succeed.
    bool initResult = ctx.Init(800, 600, "Test Window");
    REQUIRE(initResult == true);

    // Call basic window methods to ensure no crashes and increase branch
    // coverage
    REQUIRE_NOTHROW(ctx.SetWindowTitle("New Title"));
    REQUIRE_NOTHROW(ctx.PollEvents());
    REQUIRE_NOTHROW(ctx.SwapBuffers());

    // Verify closing logic state toggles
    REQUIRE(ctx.ShouldClose() == false);
    ctx.SetShouldClose(true);
    REQUIRE(ctx.ShouldClose() == true);
  }
}
