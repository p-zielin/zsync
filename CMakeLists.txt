cmake_minimum_required(VERSION 3.25)
project(zsync)

set(CMAKE_CXX_STANDARD 17)
include_directories(/usr/local/include .)
link_directories(/usr/local/lib)
add_executable(zsync main.cpp
        CService.cpp
        CService.h
        subsystem/CLoggingSubsystem.cpp
        subsystem/CZFSService.cpp
        subsystem/CZFSService.h
        entity/CDatasetInfo.cpp
        entity/CDatasetInfo.h
        entity/CBaseEntity.cpp
        entity/CBaseEntity.h
        subsystem/www/CWebServerSubsystem.cpp
        subsystem/www/CWebServerSubsystem.h
        entity/CUserInfo.cpp
        entity/CUserInfo.h
        subsystem/www/CWebRequestFactory.cpp
        subsystem/www/CWebHandler.cpp
        entity/CGroupInfo.cpp
        entity/CGroupInfo.h
        subsystem/CShellSubsystem.cpp
        subsystem/CShellSubsystem.h
        subsystem/CHelper.cpp
        subsystem/CHelper.h
        entity/CUserService.cpp
        entity/CUserService.h
        subsystem/www/CWebStorageHandler.h
        subsystem/www/CWebStorageHandler.cpp
        entity/CStorageInfo.h entity/CStorageInfo.cpp
        entity/CVirtualUser.h
        entity/CVirtualUser.cpp
        subsystem/backup/workers/CZFSReceiverWorker.cpp
        subsystem/backup/workers/CZFSReceiverWorker.h
        subsystem/backup/CBackupSubsystem.cpp
        subsystem/backup/CBackupSubsystem.h
        subsystem/backup/workers/CZFSendNotification.cpp
        subsystem/backup/workers/CZFSendNotification.h
        subsystem/backup/workers/CZFSSenderWorker.cpp
        subsystem/backup/workers/CZFSSenderWorker.h
        subsystem/backup/workers/CCleanerWorker.cpp
        subsystem/backup/workers/CCleanerWorker.h
        CByteCounter.h CByteCounter.cpp
        subsystem/backup/CTransferMonitor.h subsystem/backup/CTransferMonitor.cpp
        subsystem/backup/CTransferNotification.h subsystem/backup/CTransferNotification.cpp
        subsystem/backup/CTransferNotificationText.h subsystem/backup/CTransferNotificationText.cpp
        subsystem/backup/CTransferNotificationSend.h subsystem/backup/CTransferNotificationSend.cpp
        subsystem/backup/CTransferNotificationStop.h subsystem/backup/CTransferNotificationStop.cpp
        subsystem/backup/CTransferInfo.h subsystem/backup/CTransferInfo.cpp
        subsystem/backup/CTransferNotificationStart.h subsystem/backup/CTransferNotificationStart.cpp
        subsystem/backup/workers/CTransferSession.h subsystem/backup/workers/CTransferSession.cpp
        entity/CSnapshotInfo.h entity/CSnapshotInfo.cpp
        config/storage.ini
        subsystem/backup/workers/CWorker.cpp
        subsystem/backup/workers/CWorker.h
        subsystem/backup/workers/CBackupNotification.cpp
        subsystem/backup/workers/CBackupNotification.h
        PrivateKeyPasswordHandler.cpp
        PrivateKeyPasswordHandler.h
)

target_link_libraries(zsync PocoFoundation PocoNet PocoJSON PocoUtil PocoCrypto PocoNetSSL PocoData)
