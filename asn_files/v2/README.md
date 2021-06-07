# ASN files

This folder contains the asn files used for code generation for the message converter with `protocolVersion=2`

## Message types

| Message type | Standard          | Version |
|:-------------|:------------------|:--------|
| CAM          | ETSI EN 302 637-2 | 1.4.1   |
| DENM         | ETSI EN 302 637-3 | 1.3.1   |
| MAPEM        | ETSI TS 103 301   | 1.3.1   |
| SPATEM       | ETSI TS 103 301   | 1.3.1   |
| SREM         | ETSI TS 103 301   | 1.3.1   |
| SSEM         | ETSI TS 103 301   | 1.3.1   |

## IMPORTANT NOTE

I had to manually modify some of these asn files. This is caused by *asn1c*, as names clashes are not resolved along modules using different object identifiers. For more information check `libasn1fix/asn1fix.c` of the asn source code ([https://github.com/mouse07410/asn1c](https://github.com/mouse07410/asn1c)). The important part is here:

```C
/* resolve clash of Identifier in different modules */
int oid_exist = (tmparg.expr->module->module_oid && arg->expr->module->module_oid);
if ((!oid_exist && strcmp(tmparg.expr->module->ModuleName, arg->expr->module->ModuleName)) ||
    (oid_exist && !asn1p_oid_compare(tmparg.expr->module->module_oid, arg->expr->module->module_oid))) {

    tmparg.expr->_mark |= TM_NAMECLASH;
    arg->expr->_mark |= TM_NAMECLASH;
    continue;
}
```

Files I manually changed:

* `ISO24534-3_ElectronicRegistrationIdentificationVehicleDataModule_ForBallot.asn`:
  * Some definitions are based on `DATE` type, which is not supported by asn1c.
  * I simply commented these definitions out, as no other asn files relied on them
  * Further, some definitions clash with other modules, e.g. `VehicleHeight`, but because they were only used internally, I commented them out as well
* `ISO-TS-19091-addgrp-C-2018.asn`:
  * Some definitions clashed with the ITS-Container, I renamed them (Sufficx `Iso`).

**Because of my changes, these files should be checked when updating the files or adding new ones!**

## Sources

Find the loctions I downloaded the files from in `sources.txt`.
