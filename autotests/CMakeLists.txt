find_package(Qt6Test REQUIRED)

ecm_add_test(triggerparsingtest.cpp LINK_LIBRARIES Qt::Test KGlobalAccelD)
ecm_add_test(migrateconfigtest.cpp LINK_LIBRARIES Qt::Test KF6::ConfigCore KF6::Service KGlobalAccelD)
ecm_add_test(shortcutstest.cpp LINK_LIBRARIES Qt::Test KF6::ConfigCore KF6::Service KGlobalAccelD)
ecm_add_test(allowlisttest.cpp LINK_LIBRARIES Qt::Test KF6::ConfigCore KF6::Service KGlobalAccelD)
