#include <Eclipse/LogMacros.h>

int main() {
    LOG_INFO("APP", "Application started successfully");
    LOG_WARNING("CONFIG", "Using default configuration");
    LOG_ERROR("DATABASE", "Failed to connect to database");
    LOG_DEBUG("NETWORK", "Sending HTTP request");
    LOG_INFO_DETAILS("USER", "User logged in", "Username: john_doe");
    LOG_ERROR_DETAILS("AUTH", "Authentication failed", "Invalid credentials provided");
    
    return 0;
}
