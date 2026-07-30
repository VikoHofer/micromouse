| Command (1 byte) | Data (n byte) |
|--|--|
| `0`: printf | Debug Message (up to 249 bytes) |
| `1`: SetWall | (1 byte) - `0`: top, `1`: left, `2`: right, `3`: bottom |
| `2`: Move | (1 byte) - `0`: straight, `1`: left, `2`: right |
