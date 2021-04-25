project(CFE_IMU_APP C)

include_directories(fsw/mission_inc)
include_directories(fsw/platform_inc)

# Create the app module
add_cfe_app(imu_app fsw/src/imu_app.c)

# Include the public API from sample_lib to demonstrate how
# to call library-provided functions
add_cfe_app_dependency(imu_app mpu9dof_lib)

# Add table
add_cfe_tables(ImuAppTable fsw/tables/imu_app_tbl.c)

# If UT is enabled, then add the tests from the subdirectory
# Note that this is an app, and therefore does not provide
# stub functions, as other entities would not typically make 
# direct function calls into this application.
#  if (ENABLE_UNIT_TESTS)
#   add_subdirectory(unit-test)
# endif (ENABLE_UNIT_TESTS)
