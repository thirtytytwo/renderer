# RIDX Binary Format Specification

## Header (10 bytes)
| Offset | Size | Type     | Description       |
|--------|------|----------|-------------------|
| 0      | 4    | char[4]  | Magic "RIDX"     |
| 4      | 2    | uint16LE | Version (1)       |
| 6      | 4    | uint32LE | Section count     |

## Section Layout
| Offset | Size     | Type     | Description     |
|--------|----------|----------|-----------------|
| 0      | 1        | uint8    | Section type    |
| 1      | 4        | uint32LE | Data length    |
| 5      | variable | bytes    | Section data    |

## Section Types
| Value | Name        | Description                |
|-------|-------------|----------------------------|
| 0x01  | META        | Project metadata (kvpairs) |
| 0x02  | FILE_TREE   | File tree entries          |
| 0x03  | MODULE      | Module details             |
| 0x04  | DEP_GRAPH   | Dependency edges            |
| 0x05  | DATA_FLOW   | Data flow steps             |
| 0x06  | STATUS      | Project status              |
| 0x07  | CONVENTION  | Coding conventions           |

## Primitive Types
- **String**: uint16LE length + UTF-8 bytes
- **LineRange**: uint32LE start + uint32LE end
- **Status byte**: 0x01=ok, 0x02=warn, 0x03=error

## Section Data Formats

### META (0x01)
uint16 count; for each: String key + String value

### FILE_TREE (0x02)
uint16 count; for each: String path + String desc + LineRange + String kind
  kind: entry|header|empty|third_party|lib|resource|doc|config

### MODULE (0x03)
uint16 count; for each:
  String name + String file + String description + String callChain + String notes
  uint16 regionCount; for each: LineRange + String desc
  uint16 subModuleCount; for each:
    String name + LineRange + String description
    uint16 regionCount; for each: LineRange + String desc

### DEP_GRAPH (0x04)
uint16 count; for each: String from + String to + String type
  type: include|link

### DATA_FLOW (0x05)
uint16 count; for each: uint8 step + String description

### STATUS (0x06)
uint16 count; for each: String name + uint8 status + String desc

### CONVENTION (0x07)
uint16 count; for each: String key + String value
