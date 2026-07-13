--- CARD ---
id:        rbf-tree-structured-filesystem
type:      CONCEPT
target:    all
verify:    from-manual
topic:     rbf-storage
claim:     RBF implements a tree-structured file system designed for efficient disk space usage, resistance to accidental damage, and fast file access with relative simplicity.
context:    -
source:     Disk File Organization manual, "Basic Disk Organization"
--- END ---

--- CARD ---
id:        rbf-logical-sector-sizes
type:      FACT
target:    all
verify:    from-manual
topic:     rbf-storage
claim:     RBF supports logical sector sizes in integral binary multiples from 256 to 32,768 bytes. If the physical disk cannot directly support the logical sector size, the driver module must divide or combine physical sectors to simulate the required logical size.
context:    -
source:     Disk File Organization manual, "Basic Disk Organization"
--- END ---

--- CARD ---
id:        lsn-logical-sector-numbering
type:      CONCEPT
target:    all
verify:    from-manual
topic:     rbf-storage
claim:     OS-9 abstracts disk addresses as Logical Sector Numbers (LSNs) numbered 0 to (n-1), eliminating hardware dependencies on track, surface, and sector numbering. The disk driver or controller is responsible for mapping LSNs to physical addresses.
context:    -
source:     Disk File Organization manual, "Basic Disk Organization"
--- END ---

--- CARD ---
id:        identification-sector-lsn-zero
type:      FACT
target:    all
verify:    from-manual
topic:     rbf-storage
claim:     LSN 0 always contains the identification sector, which describes the disk's physical and logical format, allocation map size, root directory location, volume name, creation date/time, and (for bootable disks) the starting LSN and size of the OS9Boot file.
context:    -
source:     Disk File Organization manual, "Identification Sector"
--- END ---

--- CARD ---
id:        standard-disk-structure
type:      FACT
target:    all
verify:    from-manual
topic:     rbf-storage
claim:     Every OS-9 disk has the same basic structure: identification sector at LSN 0, disk allocation map usually at LSN 1, and root directory immediately following the allocation map.
context:    -
source:     Disk File Organization manual, "Basic Disk Organization"
--- END ---

--- CARD ---
id:        dd-tot-total-sectors
type:      FACT
target:    all
verify:    from-manual
topic:     rbf-identification-sector
claim:     DD_TOT field (offset $00, 3 bytes) in identification sector contains the total number of sectors on media.
context:    -
source:     Disk File Organization manual, "Identification Sector", Figure 7-1
--- END ---

--- CARD ---
id:        dd-tks-track-size
type:      FACT
target:    all
verify:    from-manual
topic:     rbf-identification-sector
claim:     DD_TKS field (offset $03, 1 byte) in identification sector specifies track size in sectors.
context:    -
source:     Disk File Organization manual, "Identification Sector", Figure 7-1
--- END ---

--- CARD ---
id:        dd-map-allocation-map-bytes
type:      FACT
target:    all
verify:    from-manual
topic:     rbf-identification-sector
claim:     DD_MAP field (offset $04, 2 bytes) in identification sector specifies the actual number of bytes used in the allocation map.
context:    -
source:     Disk File Organization manual, "Identification Sector", Figure 7-1
--- END ---

--- CARD ---
id:        dd-bit-cluster-size
type:      FACT
target:    all
verify:    from-manual
topic:     rbf-identification-sector
claim:     DD_BIT field (offset $06, 2 bytes) in identification sector specifies the number of sectors per cluster. The cluster size (sectors per cluster) is always an integral power of two.
context:    -
source:     Disk File Organization manual, "Identification Sector", Figure 7-1
--- END ---

--- CARD ---
id:        dd-dir-root-directory-lsn
type:      FACT
target:    all
verify:    from-manual
topic:     rbf-identification-sector
claim:     DD_DIR field (offset $08, 3 bytes) in identification sector contains the LSN of the root directory file descriptor.
context:    -
source:     Disk File Organization manual, "Identification Sector", Figure 7-1
--- END ---

--- CARD ---
id:        dd-own-owner-id
type:      FACT
target:    all
verify:    from-manual
topic:     rbf-identification-sector
claim:     DD_OWN field (offset $0B, 2 bytes) in identification sector contains the owner ID.
context:    -
source:     Disk File Organization manual, "Identification Sector", Figure 7-1
--- END ---

--- CARD ---
id:        dd-att-attributes
type:      FACT
target:    all
verify:    from-manual
topic:     rbf-identification-sector
claim:     DD_ATT field (offset $0D, 1 byte) in identification sector contains file attributes.
context:    -
source:     Disk File Organization manual, "Identification Sector", Figure 7-1
--- END ---

--- CARD ---
id:        dd-dsk-disk-id
type:      FACT
target:    all
verify:    from-manual
topic:     rbf-identification-sector
claim:     DD_DSK field (offset $0E, 2 bytes) in identification sector contains the disk ID.
context:    -
source:     Disk File Organization manual, "Identification Sector", Figure 7-1
--- END ---

--- CARD ---
id:        dd-fmt-disk-format
type:      FACT
target:    all
verify:    from-manual
topic:     rbf-identification-sector
claim:     DD_FMT field (offset $10, 1 byte) describes disk format with bits indicating: bit 0 = single/double side, bit 1 = FM/MFM density, bit 2 = double track (96/135 TPI), bit 3 = quad track density (192 TPI), bit 4 = octal track density (384 TPI).
context:    -
source:     Disk File Organization manual, "Identification Sector", Figure 7-1
--- END ---

--- CARD ---
id:        dd-spt-sectors-per-track
type:      FACT
target:    all
verify:    from-manual
topic:     rbf-identification-sector
claim:     DD_SPT field (offset $11, 2 bytes) in identification sector contains sectors per track as a two-byte value.
context:    -
source:     Disk File Organization manual, "Identification Sector", Figure 7-1
--- END ---

--- CARD ---
id:        dd-res-reserved
type:      FACT
target:    all
verify:    from-manual
topic:     rbf-identification-sector
claim:     DD_RES field (offset $13, 2 bytes) in identification sector is reserved for future use.
context:    -
source:     Disk File Organization manual, "Identification Sector", Figure 7-1
--- END ---

--- CARD ---
id:        dd-bt-bootstrap-lsn
type:      FACT
target:    all
verify:    from-manual
topic:     rbf-identification-sector
claim:     DD_BT field (offset $15, 3 bytes) in identification sector contains the system bootstrap LSN.
context:    -
source:     Disk File Organization manual, "Identification Sector", Figure 7-1
--- END ---

--- CARD ---
id:        dd-bsz-bootstrap-size
type:      FACT
target:    all
verify:    from-manual
topic:     rbf-identification-sector
claim:     DD_BSZ field (offset $18, 2 bytes) in identification sector specifies the size of the system bootstrap.
context:    -
source:     Disk File Organization manual, "Identification Sector", Figure 7-1
--- END ---

--- CARD ---
id:        dd-dat-creation-date
type:      FACT
target:    all
verify:    from-manual
topic:     rbf-identification-sector
claim:     DD_DAT field (offset $1A, 5 bytes) in identification sector contains the creation date in Y M D H M format.
context:    -
source:     Disk File Organization manual, "Identification Sector", Figure 7-1
--- END ---

--- CARD ---
id:        dd-nam-volume-name
type:      FACT
target:    all
verify:    from-manual
topic:     rbf-identification-sector
claim:     DD_NAM field (offset $1F, 32 bytes) in identification sector contains the volume name.
context:    -
source:     Disk File Organization manual, "Identification Sector", Figure 7-1
--- END ---

--- CARD ---
id:        dd-opt-path-descriptor-options
type:      FACT
target:    all
verify:    from-manual
topic:     rbf-identification-sector
claim:     DD_OPT field (offset $3F, 32 bytes) in identification sector contains path descriptor options.
context:    -
source:     Disk File Organization manual, "Identification Sector", Figure 7-1
--- END ---

--- CARD ---
id:        dd-sync-media-integrity-code
type:      FACT
target:    all
verify:    from-manual
topic:     rbf-identification-sector
claim:     DD_SYNC field (offset $60, 4 bytes) in identification sector contains the media integrity code.
context:    -
source:     Disk File Organization manual, "Identification Sector", Figure 7-1
--- END ---

--- CARD ---
id:        dd-maplsn-bitmap-start
type:      FACT
target:    all
verify:    from-manual
topic:     rbf-identification-sector
claim:     DD_MapLSN field (offset $64, 4 bytes) specifies the allocation map starting sector number; a value of 0 defaults to LSN 1.
context:    -
source:     Disk File Organization manual, "Identification Sector", Figure 7-1
--- END ---

--- CARD ---
id:        dd-lsnsize-logical-sector-size
type:      FACT
target:    all
verify:    from-manual
topic:     rbf-identification-sector
claim:     DD_LSNSize field (offset $68, 2 bytes) in identification sector specifies media logical sector size; a value of 0 defaults to 256 bytes.
context:    -
source:     Disk File Organization manual, "Identification Sector", Figure 7-1
--- END ---

--- CARD ---
id:        dd-versid-sector-version
type:      FACT
target:    all
verify:    from-manual
topic:     rbf-identification-sector
claim:     DD_VersID field (offset $6A, 2 bytes) in identification sector contains the Sector 0 Version ID.
context:    -
source:     Disk File Organization manual, "Identification Sector", Figure 7-1
--- END ---

--- CARD ---
id:        allocation-map-cluster-tracking
type:      FACT
target:    all
verify:    from-manual
topic:     rbf-allocation-map
claim:     Each bit in the allocation map represents a cluster on the disk. A set bit indicates the cluster is in use, defective, or non-existent. A cleared bit indicates the cluster is free.
context:    -
source:     Disk File Organization manual, "Allocation Map"
--- END ---

--- CARD ---
id:        root-directory-structure
type:      CONCEPT
target:    all
verify:    from-manual
topic:     rbf-directory
claim:     The root directory is the parent directory of all other files and directories on the disk, accessed via the physical device name (e.g., /d1). It is located at the LSN specified in DD_DIR of the identification sector.
context:    -
source:     Disk File Organization manual, "Root Directory"
--- END ---

--- CARD ---
id:        multiple-contiguous-segment-structure
type:      CONCEPT
target:    all
verify:    from-manual
topic:     rbf-file-structure
claim:     OS-9 uses multiple-contiguous-segment file structure: segments are physically contiguous sectors, and if a file cannot fit in one segment, additional segments are allocated (e.g., when expanded after creation or when sufficient contiguous free space is unavailable).
context:    -
source:     Disk File Organization manual, "Basic File Structure"
--- END ---

--- CARD ---
id:        segment-minimization-strategy
type:      BEHAVIOR
target:    all
verify:    from-manual
topic:     rbf-file-structure
claim:     File segments are kept in close physical proximity to minimize disk head movement. Small files typically have only one segment, resulting in fastest access time. It is good practice to initialize file size to maximum expected size during creation to optimize storage allocation.
context:    -
source:     Disk File Organization manual, "Basic File Structure"
--- END ---

--- CARD ---
id:        file-descriptor-sector
type:      FACT
target:    all
verify:    from-manual
topic:     rbf-file-structure
claim:     Every file has a file descriptor sector (FD) that contains a list of data segments with starting LSNs and sizes, plus metadata: file attributes, owner ID, modification time, link count, and file size. The FD is system-use only and not directly accessible to users.
context:    -
source:     Disk File Organization manual, "Basic File Structure"
--- END ---

--- CARD ---
id:        fd-att-file-attributes
type:      FACT
target:    all
verify:    from-manual
topic:     rbf-file-descriptor
claim:     FD_ATT field (offset $00, 1 byte) in file descriptor contains file attributes as permission bits: bit 7 indicates directory, bit 6 indicates non-sharable, bit 5 indicates public execute, bit 4 indicates public write, and lower bits indicate read/write permissions.
context:    -
source:     Disk File Organization manual, "Identification Sector" and surrounding text
--- END ---

--- CARD ---
id:        fd-own-owner-id
type:      FACT
target:    all
verify:    from-manual
topic:     rbf-file-descriptor
claim:     FD_OWN field (offset $01, 2 bytes) in file descriptor contains the owner's User ID.
context:    -
source:     Disk File Organization manual, "File Descriptor Content Description", Figure 7-2
--- END ---

--- CARD ---
id:        fd-dat-modification-date
type:      FACT
target:    all
verify:    from-manual
topic:     rbf-file-descriptor
claim:     FD_DAT field (offset $03, 5 bytes) in file descriptor contains the date last modified in Y M D H M format. This field updates when a file is opened in write or update mode, useful for date-dependent backups.
context:    -
source:     Disk File Organization manual, "File Descriptor Content Description", Figure 7-2
--- END ---

--- CARD ---
id:        fd-lnk-link-count
type:      FACT
target:    all
verify:    from-manual
topic:     rbf-file-descriptor
claim:     FD_LNK field (offset $08, 1 byte) in file descriptor contains the link count.
context:    -
source:     Disk File Organization manual, "File Descriptor Content Description", Figure 7-2
--- END ---

--- CARD ---
id:        fd-siz-file-size
type:      FACT
target:    all
verify:    from-manual
topic:     rbf-file-descriptor
claim:     FD_SIZ field (offset $09, 4 bytes) in file descriptor contains the file size in bytes.
context:    -
source:     Disk File Organization manual, "File Descriptor Content Description", Figure 7-2
--- END ---

--- CARD ---
id:        fd-creat-creation-date
type:      FACT
target:    all
verify:    from-manual
topic:     rbf-file-descriptor
claim:     FD_CREAT field (offset $0D, 3 bytes) in file descriptor contains the creation date in Y M D format.
context:    -
source:     Disk File Organization manual, "File Descriptor Content Description", Figure 7-2
--- END ---

--- CARD ---
id:        fd-seg-segment-list
type:      FACT
target:    all
verify:    from-manual
topic:     rbf-file-descriptor
claim:     FD_SEG field (offset $10, 240 bytes) in file descriptor contains the segment list: five-byte entries continuing until end of sector. For 256-byte sectors, this yields 48 entries. Each entry contains a 3-byte LSN and 2-byte sector count. Unused segments must be zero.
context:    -
source:     Disk File Organization manual, "File Descriptor Content Description", Figure 7-2
--- END ---

--- CARD ---
id:        segment-list-mapping
type:      BEHAVIOR
target:    all
verify:    from-manual
topic:     rbf-file-structure
claim:     The RBF file manager uses the segment list in the file descriptor to maintain file pointers, logical end-of-file, and conversion of logical file pointers to physical disk sector numbers, enabling efficient random access and record locking.
context:    -
source:     Disk File Organization manual, "Basic File Structure"
--- END ---

--- CARD ---
id:        segment-allocation-size
type:      CONCEPT
target:    all
verify:    from-manual
topic:     rbf-allocation
claim:     Each device descriptor module has a segment allocation size value specifying the minimum number of sectors to allocate to a new segment. This avoids excessive tiny segments when files expand. Higher values suit systems with large files; lower values suit systems with many small files.
context:    -
source:     Disk File Organization manual, "Segment Allocation"
--- END ---

--- CARD ---
id:        initial-file-allocation
type:      BEHAVIOR
target:    all
verify:    from-manual
topic:     rbf-allocation
claim:     When a file is created, it has no data segments allocated. Write operations past the end-of-file trigger allocation of additional sectors in minimum allocation size increments. An attempt is made to expand the last segment before adding a new segment.
context:    -
source:     Disk File Organization manual, "Segment Allocation"
--- END ---

--- CARD ---
id:        segment-truncation-strategy
type:      BEHAVIOR
target:    all
verify:    from-manual
topic:     rbf-allocation
claim:     When a file is closed, if not all allocated sectors are used, the last segment is truncated and unused sectors are deallocated in the bitmap. However, if a file is closed while not at end-of-file (in write or update mode), the last segment is not truncated, preserving excess space for random-access files. A seek(0) before close prevents loss of reserved space.
context:    "This behavior differs from typical file systems and is necessary to avoid rapid segment fragmentation in random-access databases."
source:     Disk File Organization manual, "Segment Allocation"
--- END ---

--- CARD ---
id:        directory-32-byte-entries
type:      FACT
target:    all
verify:    from-manual
topic:     rbf-directory
claim:     Directory files consist of integral 32-byte entries. Each entry has a 28-byte file name field (DIR_NM, bytes 0-27) with the sign bit of the last character set. The first byte is zero for deleted/unused entries. Byte 28 is unused (must be zero). Bytes 29-31 contain the 3-byte LSN of the file's FD sector (DIR_FD).
context:    -
source:     Disk File Organization manual, "Directory File Format"
--- END ---

--- CARD ---
id:        dot-and-double-dot-entries
type:      BEHAVIOR
target:    all
verify:    from-manual
topic:     rbf-directory
claim:     When a directory file is created, two entries are automatically created: dot (.) referring to the directory itself and double-dot (..) referring to its parent directory.
context:    -
source:     Disk File Organization manual, "Directory File Format"
--- END ---

--- CARD ---
id:        raw-physical-io-via-at-suffix
type:      CONCEPT
target:    all
verify:    from-manual
topic:     rbf-io
claim:     A device can be opened for raw physical I/O by appending the @ character to the device name (e.g., /d2@). Standard open/close/read/write/seek system calls apply. This allows byte/sector access by physical address, bypassing the normal file system.
context:    "Used for diagnostic and utility programs; bypasses file security."
source:     Disk File Organization manual, "Raw Physical I/O on RBF Devices"
--- END ---

--- CARD ---
id:        raw-io-seek-calculation
type:      BEHAVIOR
target:    all
verify:    from-manual
topic:     rbf-io
claim:     For raw physical I/O, seek to address = LSN × logical sector size. To find logical sector size, read PD_SctSiz from the path descriptor; if 0, assume 256 bytes. Example: for 1024-byte sectors, seek to address 3072 to read sector 3.
context:    -
source:     Disk File Organization manual, "Raw Physical I/O on RBF Devices"
--- END ---

--- CARD ---
id:        raw-io-security-restrictions
type:      BEHAVIOR
target:    all
verify:    from-manual
topic:     rbf-io
claim:     Only super-users can open the raw device for write. Non-super-users can only read the identification sector (LSN 0) and the allocation bitmap; attempts to read past this return end-of-file error. Raw physical I/O bypasses file security entirely.
context:    -
source:     Disk File Organization manual, "Raw Physical I/O on RBF Devices"
--- END ---

--- CARD ---
id:        record-locking-concept
type:      CONCEPT
target:    all
verify:    from-manual
topic:     rbf-locking
claim:     Record locking preserves file integrity when multiple processes access the same file. RBF implements conflict detection and prevention: detecting when a record is being modified and deferring reads until the record is safe.
context:    -
source:     Disk File Organization manual, "Record Locking"
--- END ---

--- CARD ---
id:        byte-level-record-locking
type:      FACT
target:    all
verify:    from-manual
topic:     rbf-locking
claim:     RBF provides true record locking on a byte basis. When a file is opened in update mode, any read operation locks out records starting at the current file pointer for the number of bytes requested. The locked region remains locked until the next read, write, or close.
context:    -
source:     Disk File Organization manual, "Record Locking and Unlocking"
--- END ---

--- CARD ---
id:        read-mode-no-locking
type:      BEHAVIOR
target:    all
verify:    from-manual
topic:     rbf-locking
claim:     Reading files opened in read or execute mode does not cause record locking, since records cannot be updated in these modes.
context:    -
source:     Disk File Organization manual, "Record Locking and Unlocking"
--- END ---

--- CARD ---
id:        single-locked-region
type:      GOTCHA
target:    all
verify:    from-manual
topic:     rbf-locking
claim:     Only one portion of a file may be locked at a time. To lock multiple regions, open multiple paths to the same file; RBF detects same-process ownership and prevents mutual lockout. Alternatively, lock the entire file before updating multiple records.
context:    -
source:     Disk File Organization manual, "Record Locking and Unlocking"
--- END ---

--- CARD ---
id:        non-sharable-file-bit
type:      FACT
target:    all
verify:    from-manual
topic:     rbf-locking
claim:     The single-user (S) bit in the file attribute byte marks a file as non-sharable, preventing more than one process from opening it. Setting this bit during file creation or later using the attr utility triggers error #253 if another process attempts to open the file.
context:    -
source:     Disk File Organization manual, "Non-Sharable Files"
--- END ---

--- CARD ---
id:        non-sharable-dup-quirk
type:      GOTCHA
target:    all
verify:    from-manual
topic:     rbf-locking
claim:     A non-sharable file can be duplicated via I$Dup or inherited by child processes, potentially making it accessible to multiple processes despite the non-sharable attribute. It is usually a bad idea to have two processes actively using a disk file through the same inherited path.
context:    -
source:     Disk File Organization manual, "Non-Sharable Files"
--- END ---

--- CARD ---
id:        eof-lock-behavior
type:      BEHAVIOR
target:    all
verify:    from-manual
topic:     rbf-locking
claim:     EOF (end-of-file) lock occurs when reading or writing at the end of file. The EOF lock is held until a read or write occurs that is not at end-of-file. EOF lock is the only time a write automatically locks out any part of the file. This prevents simultaneous file extension by multiple processes.
context:    -
source:     Disk File Organization manual, "End of File Lock"
--- END ---

--- CARD ---
id:        eof-lock-sequential-output
type:      BEHAVIOR
target:    all
verify:    from-manual
topic:     rbf-locking
claim:     When a program creates a file for sequential output, EOF lock is immediately gained, preventing other processes from passing the writer. This enables spooler utilities to begin reading an output file before the writer finishes.
context:    -
source:     Disk File Organization manual, "End of File Lock"
--- END ---

--- CARD ---
id:        deadlock-detection
type:      CONCEPT
target:    all
verify:    from-manual
topic:     rbf-locking
claim:     Deadlock occurs when two processes attempt to gain control of the same two disk areas simultaneously, each blocking the other. RBF detects this and returns error #254 to the process causing the deadlock.
context:    -
source:     Disk File Organization manual, "Deadlock Detection"
--- END ---

--- CARD ---
id:        deadlock-avoidance-strategy
type:      BEHAVIOR
target:    all
verify:    from-manual
topic:     rbf-locking
claim:     To avoid deadlock, access records of shared files in the same sequence across all concurrent processes. For example, always read the index file before the data file, never the reverse. Simply retrying a deadlocked operation is ineffective; at least one process must release its control for others to proceed.
context:    -
source:     Disk File Organization manual, "Deadlock Detection"
--- END ---

--- CARD ---
id:        open-create-update-mode-guideline
type:      BEHAVIOR
target:    all
verify:    from-manual
topic:     rbf-locking
claim:     Do not open a file for update if only reading is intended. Files opened read-only do not cause record locking and generally run faster. On multi-user systems, files routinely opened for update on shared files can cause extended record-lock contention.
context:    -
source:     Disk File Organization manual, "Record Locking Details for I/O Functions"
--- END ---

--- CARD ---
id:        readline-lock-behavior
type:      BEHAVIOR
target:    all
verify:    from-manual
topic:     rbf-locking
claim:     Read and ReadLine operations cause record lock-out only if the file is opened in update mode. The locked region includes all bytes from current file pointer for the requested byte count. A ReadLine requesting 256 bytes locks exactly 256 bytes regardless of actual bytes read before carriage return.
context:    -
source:     Disk File Organization manual, "Record Locking Details for I/O Functions"
--- END ---

--- CARD ---
id:        record-lock-release-conditions
type:      BEHAVIOR
target:    all
verify:    from-manual
topic:     rbf-locking
claim:     A locked record is released by: another read operation, a write operation, file close, or a record lock SetStat call. Any read or write of zero bytes releases any record lock, EOF lock, or file lock.
context:    -
source:     Disk File Organization manual, "Record Locking Details for I/O Functions"
--- END ---

--- CARD ---
id:        write-release-behavior
type:      BEHAVIOR
target:    all
verify:    from-manual
topic:     rbf-locking
claim:     Write and WriteLine calls always release any currently locked record. A write of zero bytes releases EOF lock and file lock. Writing usually does not lock any portion of the file unless it occurs at end-of-file, which gains EOF lock.
context:    -
source:     Disk File Organization manual, "Record Locking Details for I/O Functions"
--- END ---

--- CARD ---
id:        seek-no-effect-locking
type:      FACT
target:    all
verify:    from-manual
topic:     rbf-locking
claim:     Seek operations do not affect record locking.
context:    -
source:     Disk File Organization manual, "Record Locking Details for I/O Functions"
--- END ---

--- CARD ---
id:        setstatus-lock-operations
type:      FACT
target:    all
verify:    from-manual
topic:     rbf-locking
claim:     Two SetStat codes deal with record locking: SS_Lock locks or releases part of a file; SS_Ticks sets the length of time a program will wait for a locked record.
context:    -
source:     Disk File Organization manual, "Record Locking Details for I/O Functions"
--- END ---

--- CARD ---
id:        file-security-ownership
type:      CONCEPT
target:    all
verify:    from-manual
topic:     rbf-security
claim:     Each file has a group/user ID identifying the owner, copied from the process descriptor when the file is created. File security is enforced through the owner ID and attribute byte, specifying read/write/execute access for owner and public (different group ID).
context:    -
source:     Disk File Organization manual, "File Security"
--- END ---

--- CARD ---
id:        access-control-per-path
type:      BEHAVIOR
target:    all
verify:    from-manual
topic:     rbf-security
claim:     Whenever a file is opened, access permissions are checked on all directories in the pathlist and the file itself. Without read permission on a directory, no files in that directory are accessible.
context:    -
source:     Disk File Organization manual, "File Security"
--- END ---

--- CARD ---
id:        superuser-access
type:      FACT
target:    all
verify:    from-manual
topic:     rbf-security
claim:     Any super-user (group ID of zero) may access any file. Files owned by super-user cannot be accessed by other groups unless specific permissions are set. Module files owned by super-user must also be owned by super-user or the modules are not loaded.
context:    -
source:     Disk File Organization manual, "File Security"
--- END ---

--- CARD ---
id:        fd-own-byte-limit-caveat
type:      GOTCHA
target:    all
verify:    from-manual
topic:     rbf-security
claim:     FD_OWN is a 2-byte field, but RBF only reads the low-order byte of both group ID and user ID from the password file. A user with ID 256.512 is mistaken for the super-user by RBF.
context:    "The password file permits 2 bytes for group ID and 2 bytes for user ID, but RBF truncates to 1 byte per field."
source:     Disk File Organization manual, "File Security"
--- END ---

--- CARD ---
id:        at-special-file-locking-caveat
type:      GOTCHA
target:    all
verify:    from-manual
topic:     rbf-io
claim:     The special "@" raw device file is treated as a different file from the normal device name for record-locking purposes. Record lock conflicts are only checked between processes using the same "@" device path. Use extreme care opening "@" in update mode since record locking overhead is kept low.
context:    -
source:     Disk File Organization manual, "Raw Physical I/O on RBF Devices" and "Record Locking Details for I/O Functions"
--- END ---

