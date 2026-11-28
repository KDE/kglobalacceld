find_package(Qt6Test REQUIRED)

if(CMAKE_VERSION VERSION_GREATER_EQUAL 3.29) # we have cmake_minimum_required(VERSION 3.16) at the time of writing
    find_program(DBUS_RUN_SESSION dbus-run-session)
    if(DBUS_RUN_SESSION)
        set(CMAKE_TEST_LAUNCHER "${DBUS_RUN_SESSION}")
    endif()
endif()
ecm_add_test(shortcutstest.cpp LINK_LIBRARIES Qt::Test KF6::ConfigCore KF6::Service KGlobalAccelD ENVIRONMENT "QT_QPA_PLATFORM=offscreen")
ecm_add_test(allowlisttest.cpp LINK_LIBRARIES Qt::Test KF6::ConfigCore KF6::Service KGlobalAccelD ENVIRONMENT "QT_QPA_PLATFORM=offscreen")
unset(CMAKE_TEST_LAUNCHER)

ecm_add_test(componenttest.cpp LINK_LIBRARIES Qt::Test KF6::ConfigCore KGlobalAccelD)
ecm_add_test(migrateconfigtest.cpp LINK_LIBRARIES Qt::Test KF6::ConfigCore KF6::Service KGlobalAccelD)
