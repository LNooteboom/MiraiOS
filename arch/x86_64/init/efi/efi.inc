%ifndef EFI_INC
%define EFI_INC

STRUC efiTableHeader
	.signature:		resq 1
	.revision:		resd 1
	.headerSize:	resd 1
	.crc32:			resd 1
	.reserved:		resd 1
	.size:
ENDSTRUC

STRUC efiSysTable
	.header:			resb efiTableHeader.size
	.firmwareVendor:	resq 1
	.firmwareRevision:	resq 1

	.conInHandle:		resq 1
	.conIn:				resq 1
	.conOutHandle:		resq 1
	.conOut:			resq 1
	.stderrHandle:		resq 1
	.stderr:			resq 1

	.runtimeServices:	resq 1
	.bootServices:		resq 1

	.nrofTableEntries:	resq 1
	.configurationTable: resq 1
ENDSTRUC

STRUC efiBootTable
	.header:			resb efiTableHeader.size
	.raiseTPL:			resq 1
	.restoreTPL:		resq 1
	.allocatePages:		resq 1
	.freePages:			resq 1
	.getMemoryMap:		resq 1
	.allocatePool:		resq 1
	.freePool:			resq 1
	.createEvent:		resq 1
	.setTimer:			resq 1
	.waitForEvent:		resq 1
	.signalEvent:		resq 1
	.closeEvent:		resq 1
	.checkEvent:		resq 1
	.installProtocolInterface: resq 1
	.reinstallProtocolInterface: resq 1
	.uninstallProtocolInterface: resq 1
	.handleProtocol:	resq 1
	.reserved:			resq 1
	.registerProtocolNotify: resq 1
	.locateHandle:		resq 1
	.locateDevicePath:	resq 1
	.installConfigurationTable: resq 1
	.loadImage:			resq 1
	.startImage:		resq 1
	.exit:				resq 1
	.unloadImage:		resq 1
	.exitBootServices:	resq 1
	.getNextMonotonicCount:	resq 1
	.stall:				resq 1
	.setWatchdogTimer:	resq 1
	.connectController:	resq 1
	.disconnectController: resq 1
	.openProtocol:		resq 1
	.closeProtocol:		resq 1
	.openProtocolInformation: resq 1
	.protocolsPerHandle: resq 1
	.locateHandleBuffer: resq 1
	.locateProtocol:	resq 1
	.installMultipleProtocolInterfaces: resq 1
	.uninstallMultipleProtocolInterfaces: resq 1
	.calculateCRC32:	resq 1
	.copyMem:			resq 1
	.setMem:			resq 1
	.createEventEx:		resq 1
ENDSTRUC

STRUC efiMemDescriptor
	.type:				resq 1
	.physicalStart:		resq 1
	.virtualStart:		resq 1
	.nrofPages:			resq 1
	.attributes:		resq 1
ENDSTRUC

%endif