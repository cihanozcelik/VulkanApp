#include "core/Application.h"

#include <cstdlib>
#include <iostream>
#include <stdexcept>

int main()
{
  // Create the main application object
  Application app;

  try
  {
    // Run the application
    app.Run();
  }
  catch (const std::exception& e)
  {
    // Catch and report any standard exceptions
    std::cerr << "FATAL ERROR: " << e.what() << std::endl;
    return EXIT_FAILURE;
  }
  catch (...)
  {
      // Catch any other unknown exceptions
      std::cerr << "FATAL ERROR: Unknown exception caught!" << std::endl;
      return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
} 