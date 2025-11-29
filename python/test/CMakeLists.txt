if(BUILD_TESTING)
  if (Pytest_AVAILABLE)
    include("../Pytest.cmake")

    if (NOT DISCOVER_TESTS_AFTER_BUILD OR APPEND_TESTS_ONLY)
      pytest_discover_tests(PythonBindings)
    endif()
  endif()
endif()

