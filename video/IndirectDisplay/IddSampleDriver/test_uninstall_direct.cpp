#include "vddsdk.h"
#include <iostream>
#include <Windows.h>

int main() {
    std::cout << "=== Direct UninstallDriver Test ===" << std::endl;
    
    // Check admin
    BOOL isAdmin = FALSE;
    PSID adminGroup = NULL;
    SID_IDENTIFIER_AUTHORITY ntAuthority = SECURITY_NT_AUTHORITY;
    
    if (AllocateAndInitializeSid(&ntAuthority, 2, SECURITY_BUILTIN_DOMAIN_RID,
                                  DOMAIN_ALIAS_RID_ADMINS, 0, 0, 0, 0, 0, 0, &adminGroup)) {
        CheckTokenMembership(NULL, adminGroup, &isAdmin);
        FreeSid(adminGroup);
    }
    
    std::cout << "Running as admin: " << (isAdmin ? "YES" : "NO") << std::endl;
    std::cout << std::endl;
    
    std::cout << "Calling vdd::UninstallDriver()..." << std::endl;
    vdd::Status status = vdd::UninstallDriver();
    std::cout << "Result: " << (int)status << std::endl;
    std::cout << "Last error: " << vdd::GetLastError() << std::endl;
    
    return 0;
}

