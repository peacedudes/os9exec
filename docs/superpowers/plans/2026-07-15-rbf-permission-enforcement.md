# RBF Permission Enforcement Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Make RBF file/directory permission bits actually enforced (owner vs public, super-user bypass), and stamp real ownership on file/directory creation — both currently no-ops (`ROADMAP.md`, "File permissions and ownership don't work").

**Architecture:** All work is in `Source/OS9exec_core/file_rbf.c`. `Create_FD`/`CreateNewFile` gain an owner-word parameter so new files/directories are stamped with their creator's `group.user` instead of staying `0.0`. A small permission primitive (`has_perm`/`has_open_perm`/`IsOwner`) is wired into the single choke point every RBF path-resolution op already shares — `pRopen`'s directory-walk loop — plus `pRdelete` (parent-directory write check) and `pRsetatt` (owner-only attribute changes). `is_super()` (already exists, `procstuff.c:220`) bypasses every check, so the entire existing test suite (which always runs as super-user) sees zero behavior change — only a test that explicitly `login`s as a non-super account can exercise the new code at all.

**Tech Stack:** C (os9exec core), Swift (integration test suite in `test/Sources/OS9Tests/main.swift`).

**Design doc:** `docs/superpowers/specs/2026-07-15-rbf-permission-enforcement-design.md` — read it for the full rationale (why read-only gates directory traversal, why no chown, why `E_FNA`, etc.) before starting.

## Global Constraints

- **RBF images only.** Host-native directories (`os9exec`'s directory-shim devices) must be completely unaffected — no permission bits to enforce there, and this plan does not touch that code path at all.
- **No `chown`.** Ownership is set once, at creation, and never changes — matches real OS-9 (no `SS_Own` SetStat code exists anywhere in this codebase today).
- **Directory traversal is read-only-gated**, no Unix-style directory-execute check — matches the documented real-RBF model, not Unix. A directory's execute bit is never consulted.
- **Permission-denied returns `E_FNA`** everywhere (open, create, delete, setattr) — the documented real-OS-9 code for "access permissions not satisfied," already used elsewhere in `pRopen`'s error paths.
- **Every existing test must stay green.** The whole suite runs as super-user; `is_super()` bypass is what guarantees this. Re-run `make test` after every code-change task.

---

### Task 1: Stamp real ownership on file/directory creation

**Files:**
- Modify: `Source/OS9exec_core/file_rbf.c:2083-2101` (accessor family: add `FDOwn`/`Set_FDOwn` next to `FDAtt`/`Set_FDAtt`)
- Modify: `Source/OS9exec_core/file_rbf.c:2621-2634` (`Create_FD` — add owner parameter)
- Modify: `Source/OS9exec_core/file_rbf.c:2762-2803` (`CreateNewFile` — add `pid` parameter, compute+pass owner word)
- Modify: `Source/OS9exec_core/file_rbf.c:2954-2955` (the one call site inside `pRopen`)

**Interfaces:**
- Produces: `static ushort FDOwn(syspath_typ* spP)`, `static void Set_FDOwn(syspath_typ* spP, ushort owner)` — read/write the packed `group<<8|user` owner word at FD sector offset `$01` (2 bytes), using the existing `GET_OS9W`/`SET_OS9W` macros (`os9_ll.h:184-185`). Later tasks (2, 3) call `FDOwn`.
- Produces: `static os9err Create_FD(syspath_typ* spP, byte att, ushort owner, ulong size)` — signature gains `owner` between `att` and `size`.
- Produces: `static os9err CreateNewFile(ushort pid, syspath_typ* spP, byte fileAtt, char* name, ulong csize)` — signature gains `pid` as the first parameter.
- Consumes: `procs[pid].pd._group`, `procs[pid].pd._user` (existing fields, already used this way at `fcalls.c:702`), `BpB` (`=8`, `os9exec_nt.h:346`).

- [ ] **Step 1: Add the `FDOwn`/`Set_FDOwn` accessors**

  In `file_rbf.c`, immediately after `Set_FDAtt` (ends at line 2096, right before `Set_FDLnk`):

  ```c
  static ushort FDOwn( syspath_typ* spP )
  /* get the file owner: packed group.user, group<<8|user */
  {   return GET_OS9W(spP->fd_sct, 1);
  } /* FDOwn */

  static void Set_FDOwn( syspath_typ* spP, ushort owner )
  /* set the file owner: packed group.user, group<<8|user */
  {   SET_OS9W(spP->fd_sct, 1, owner);
  } /* Set_FDOwn */
  ```

- [ ] **Step 2: Thread an owner parameter through `Create_FD`**

  Change (line 2621):
  ```c
  static os9err Create_FD( syspath_typ* spP, byte att, ulong size )
  {
      rbfdev_typ* dev= &rbfdev[spP->u.rbf.devnr];
      
      int  ii;
      for (ii=0; ii<dev->sctSize; ii++) { /* clear sector */
            spP->fd_sct[ii]= NUL;
      }

      Set_FDAtt     ( spP,  att ); /* attributes */
      Set_FDLnk     ( spP,    1 ); /* the link count */
      Set_FDSize    ( spP, size ); /* file size  */
      return WriteFD( spP );       /* write FD sector */
  } /* Create_FD */
  ```
  to:
  ```c
  static os9err Create_FD( syspath_typ* spP, byte att, ushort owner, ulong size )
  {
      rbfdev_typ* dev= &rbfdev[spP->u.rbf.devnr];
      
      int  ii;
      for (ii=0; ii<dev->sctSize; ii++) { /* clear sector */
            spP->fd_sct[ii]= NUL;
      }

      Set_FDAtt     ( spP,  att   ); /* attributes */
      Set_FDOwn     ( spP,  owner ); /* creator's group.user */
      Set_FDLnk     ( spP,    1   ); /* the link count */
      Set_FDSize    ( spP, size   ); /* file size  */
      return WriteFD( spP );         /* write FD sector */
  } /* Create_FD */
  ```

- [ ] **Step 3: Thread `pid` through `CreateNewFile` and compute the owner word**

  Change the signature (line 2762) from:
  ```c
  static os9err CreateNewFile( syspath_typ* spP, byte fileAtt, char* name, ulong csize )
  {
      os9err      err;
      rbf_typ*    rbf= &spP->u.rbf;
      rbfdev_typ* dev= &rbfdev[rbf->devnr];
      ulong       sct=  dev->sctSize;
      ulong       clu=  dev->clusterSize;
      ulong       dfd=  rbf->fd_nr;
      uint32_t*   d  = &rbf->deptr;
      ulong       fd, scs, ascs, sTmp;
  ```
  to:
  ```c
  static os9err CreateNewFile( ushort pid, syspath_typ* spP, byte fileAtt, char* name, ulong csize )
  {
      os9err      err;
      rbf_typ*    rbf= &spP->u.rbf;
      rbfdev_typ* dev= &rbfdev[rbf->devnr];
      ulong       sct=  dev->sctSize;
      ulong       clu=  dev->clusterSize;
      ulong       dfd=  rbf->fd_nr;
      uint32_t*   d  = &rbf->deptr;
      ulong       fd, scs, ascs, sTmp;
      ushort      owner= (ushort)( (os9_word(procs[pid].pd._group)<<BpB) | os9_word(procs[pid].pd._user) );
  ```

  **Note:** `pd._group`/`pd._user` are stored in the guest's raw (big-endian) process-descriptor layout, not host-native — every other place in this codebase that combines them into one value wraps both in `os9_word()` first (see `fcalls.c:702`). Omitting the wrap here silently corrupts the packed word on a little-endian host (this was caught in task review, not in the original plan draft — the plan draft omitted `os9_word()` and got it wrong).

  Then change the `Create_FD` call site inside the same function (line 2784) from:
  ```c
      err= Create_FD      ( spP,         fileAtt, 0 ); if (err) return err;
  ```
  to:
  ```c
      err= Create_FD      ( spP,         fileAtt, owner, 0 ); if (err) return err;
  ```

- [ ] **Step 4: Update the one caller, inside `pRopen`**

  Change (around line 2953-2956):
  ```c
                  if (cre && strcmp( p,"" )==0) {            /* create it ? */
                      err= CreateNewFile( spP, procs[pid].fileAtt,
                            (char*)&cmp_entry, procs[pid].cre_initsize );
                      rbf->currPos= 0;  /* initialize position to 0 */
                      rbf->lastPos= 0;
                  }
  ```
  to:
  ```c
                  if (cre && strcmp( p,"" )==0) {            /* create it ? */
                      err= CreateNewFile( pid, spP, procs[pid].fileAtt,
                            (char*)&cmp_entry, procs[pid].cre_initsize );
                      rbf->currPos= 0;  /* initialize position to 0 */
                      rbf->lastPos= 0;
                  }
  ```

  `pRmakdir` needs no separate change — it creates its directory entry by calling `usrpath_open(...,poCreateMask)`, which resolves through this exact same `pRopen` → `CreateNewFile` path, so it inherits owner-stamping automatically.

- [ ] **Step 5: Build clean**

  Run: `make`
  Expected: builds with no new warnings or errors. (`Create_FD` and `CreateNewFile` are `static` with a single call site each, already updated — no dangling references.)

- [ ] **Step 6: Run the full existing suite — confirm zero regressions**

  Run: `swift run --package-path test`
  Expected: same pass count as before this change (ownership stamping alone has no enforcement wired up yet, so it's a pure data-model addition — nothing should be able to observe a behavior difference yet).

- [ ] **Step 7: Commit**

  ```bash
  git add Source/OS9exec_core/file_rbf.c
  git commit -m "Fix: RBF now stamps the creator's group.user as FD_OWN on new files/dirs"
  ```

---

### Task 2: Write the enforcement tests (expected to fail — no enforcement exists yet)

This replaces the pinned `fs: KNOWN-LIMITATION permissions not enforced` test with real behavioral assertions. Written and run *before* Task 3's enforcement code exists, so the "should now be blocked" assertions are expected to fail here — that failure is the proof the tests actually exercise something, not a mistake to fix in this task.

**Files:**
- Modify: `test/Sources/OS9Tests/main.swift:683-701` (replace the `KNOWN-LIMITATION` block)

**Interfaces:**
- Consumes: `run(_:expectation:commands:disk:check:)`, `check(_:contains:_:)`, `check(_:absent:_:)` (existing helpers, `main.swift:188-220`). `repoRoot` (`main.swift:33`) for host-side cleanup of `mount -k` scratch images, matching the pattern already used at `main.swift:553-587`.
- Consumes accounts already present in this suite's default disk (`repoRoot/h0/SYS/password`, live-verified via the `os9-dev` skill's REPL notes): `claude` (group.user `1.7`, blank password), `dog` (group.user `1.3`, blank password) — both non-super, both loggable-in with a bare `login claude` / `login dog` (empty password line, no interactive prompt hang). `su` (`0.0`) is simply the suite's default top-level session — no login needed to exercise the super-user bypass.

- [ ] **Step 1: Replace the KNOWN-LIMITATION block**

  In `test/Sources/OS9Tests/main.swift`, replace lines 683-701 (from the `// ---- permissions: NOT enforced ...` comment through the final `try? FileManager...removeItem(atPath: ...permDev...)`) with:

  ```swift
  // ---- permissions: ENFORCED on RBF images (real ownership + attribute bits) ----
  // claude (1.7) and dog (1.3) are blank-password non-super accounts already in
  // this disk's /dd/SYS/password -- see os9-dev skill's REPL notes for how this
  // was live-verified (login really does F$SUser to a distinct Grp.Usr). su (0.0)
  // is just this suite's default top-level session -- no login needed for it.
  let permDev     = "h8"
  let permDevPath = "/\(permDev)"
  try? FileManager.default.removeItem(atPath: repoRoot.appendingPathComponent(permDev).path)

  run("fs: permission — claude creates+owns a file, can read it back",
      expectation: "dump of a freshly created file shows its bytes to its own creator",
      commands: ["mount -k=200K \(permDev)", "chd \(permDevPath)",
                 "login claude", "chd \(permDevPath)",
                 "echo abc >f", "dump f"]) { $0.contains("6162 63") }

  check("fs: permission — owner locks self out by clearing owner-read",
      contains: "Error #",
      "chd \(permDevPath)", "login claude", "chd \(permDevPath)",
      "attr f -nr", "dump f")

  check("fs: permission — super-user still bypasses every check",
      contains: "6162 63",
      "chd \(permDevPath)", "dump f")

  check("fs: permission — non-owner (dog) blocked from a file with no public bits",
      contains: "Error #",
      "chd \(permDevPath)", "login dog", "chd \(permDevPath)", "dump f")

  check("fs: permission — owner restores public-read, non-owner can now read",
      contains: "6162 63",
      "chd \(permDevPath)", "login claude", "chd \(permDevPath)", "attr f -pr", "logout",
      "login dog", "chd \(permDevPath)", "dump f")

  check("fs: permission — non-owner cannot change attributes on a file they don't own",
      contains: "Error #",
      "chd \(permDevPath)", "login dog", "chd \(permDevPath)", "attr f -nr")

  try? FileManager.default.removeItem(atPath: repoRoot.appendingPathComponent(permDev).path)

  // ---- permissions: directories (read gates traversal, write gates create/delete) ----
  let dirPermDev  = "ha"
  let dirPermPath = "/\(dirPermDev)"
  try? FileManager.default.removeItem(atPath: repoRoot.appendingPathComponent(dirPermDev).path)

  run("fs: permission dir — dog can create inside claude's dir while public r+w are set",
      expectation: "makdir's default attrs (owner+public rwx) let a non-owner create inside",
      commands: ["mount -k=200K \(dirPermDev)", "chd \(dirPermPath)",
                 "login claude", "chd \(dirPermPath)", "makdir sub", "logout",
                 "login dog", "chd \(dirPermPath)/sub", "echo x >g"]) { !$0.contains("Error #") }

  check("fs: permission dir — write blocked once public-write is cleared",
      contains: "Error #",
      "chd \(dirPermPath)", "login claude", "chd \(dirPermPath)", "attr sub -npw", "logout",
      "login dog", "chd \(dirPermPath)/sub", "echo x >g2")

  check("fs: permission dir — directory itself unreachable once public-read is also cleared",
      contains: "Error #",
      "chd \(dirPermPath)", "login claude", "chd \(dirPermPath)", "attr sub -npr", "logout",
      "login dog", "chd \(dirPermPath)/sub")

  try? FileManager.default.removeItem(atPath: repoRoot.appendingPathComponent(dirPermDev).path)

  // ---- host-native devices: unaffected -- no real permission bits to enforce ----
  check("fs: permission — host-native device ignores attribute bits (unchanged behavior)",
      contains: "6162 63",
      "mount -k=0 hb", "chd /hb", "echo abc >f", "attr f -nr -nw -ne -npr -npw -npe", "dump f")
  try? FileManager.default.removeItem(atPath: repoRoot.appendingPathComponent("hb").path)
  ```

- [ ] **Step 2: Run just these new tests and confirm the expected split**

  Run: `swift run --package-path test "fs: permission"`

  Expected: the "creates+owns", "super-user still bypasses", "makdir...public r+w", and "host-native device ignores" tests **PASS** (nothing in Task 1 changed observable behavior for these). The "owner locks self out", "non-owner blocked", "owner restores public-read...non-owner can now read" (this one currently passes too, trivially, since nothing blocks it yet — that's fine), "non-owner cannot change attributes", "write blocked once public-write cleared", and "directory itself unreachable" tests are expected to **FAIL** — there is no enforcement code yet, so nothing is actually being blocked. This split is the point: it proves these assertions exercise real behavior, not a tautology.

- [ ] **Step 3: Commit**

  ```bash
  git add test/Sources/OS9Tests/main.swift
  git commit -m "Tests: flip the RBF permission KNOWN-LIMITATION test into real enforcement tests"
  ```

---

### Task 3: Wire permission enforcement into pRopen/pRdelete/pRsetatt

**Files:**
- Modify: `Source/OS9exec_core/file_rbf.c` (new helpers near line 2101, then edits inside `pRopen` ~2814-3034, `pRdelete` ~3155-3189, `pRsetatt` ~3362-3367)

**Interfaces:**
- Consumes: `FDAtt`, `FDOwn`, `is_super(ushort pid)` (`procstuff.c:220`), `IsRead`/`IsWrite`/`IsExec` (`utilstuff.c:845-863`, already used elsewhere in `pRopen` for `IsWrite`/`IsExec`/`IsCrea`), `OpenDir`/`CloseDir` (existing helpers, `file_rbf.c:2636-2671`, already used by `Access_DirEntry` for exactly this "peek at another directory's FD sector" pattern).
- Produces: `static Boolean IsOwner(ushort pid, ushort ownerWord)`, `static Boolean has_perm(ushort pid, byte att, ushort ownerWord, perm_typ want)`, `static Boolean has_open_perm(ushort pid, byte att, ushort ownerWord, ushort mode)` — used only within this task; no other file needs them.

- [ ] **Step 1: Add the permission primitives**

  In `file_rbf.c`, immediately after the `Set_FDOwn` accessor added in Task 1 (right before `Set_FDLnk`), add:

  ```c
  typedef enum { permRead, permWrite, permExec } perm_typ;

  static Boolean IsOwner( ushort pid, ushort ownerWord )
  /* true if the caller's group.user matches the file's owner word */
  {
      ushort caller= (ushort)( (os9_word(procs[pid].pd._group)<<BpB) | os9_word(procs[pid].pd._user) );
      return caller==ownerWord;
  } /* IsOwner */

  static Boolean has_perm( ushort pid, byte att, ushort ownerWord, perm_typ want )
  /* single-bit permission test: super-user always passes; otherwise pick the
   * owner or public bit-triplet depending on ownership, and test one bit. */
  {
      byte ownerBit, publicBit;

      if (is_super(pid)) return true;

      switch (want) {
        case permRead : ownerBit= 0x01; publicBit= 0x08; break;
        case permWrite: ownerBit= 0x02; publicBit= 0x10; break;
        default       : ownerBit= 0x04; publicBit= 0x20; break; /* permExec */
      } /* switch */

      return (att & (IsOwner(pid,ownerWord) ? ownerBit : publicBit)) != 0;
  } /* has_perm */

  static Boolean has_open_perm( ushort pid, byte att, ushort ownerWord, ushort mode )
  /* every access type actually requested by <mode> (R/W/E) must be granted */
  {
      if (IsRead (mode) && !has_perm(pid,att,ownerWord,permRead )) return false;
      if (IsWrite(mode) && !has_perm(pid,att,ownerWord,permWrite)) return false;
      if (IsExec (mode) && !has_perm(pid,att,ownerWord,permExec )) return false;
      return true;
  } /* has_open_perm */
  ```

- [ ] **Step 2: Check read-permission on the root directory when it's the final target**

  In `pRopen`, change (around line 2926-2932):
  ```c
              if (root) { 
                  strcpy( spP->name,pathname+1 );
                  err= FD_Segment( spP, &attr,&size,&totsize,&sect,&slim, &pref ); if (err) break;
                  rbf->lastPos= size;                   /* last pos is the filesize */
                  rbf->att    = attr;                   /* save attributes */
                  return 0; 
              } // if
  ```
  to:
  ```c
              if (root) { 
                  strcpy( spP->name,pathname+1 );
                  err= FD_Segment( spP, &attr,&size,&totsize,&sect,&slim, &pref ); if (err) break;
                  rbf->lastPos= size;                   /* last pos is the filesize */
                  rbf->att    = attr;                   /* save attributes */
                  if (!has_perm( pid, attr, FDOwn(spP), permRead )) return E_FNA;
                  return 0; 
              } // if
  ```

- [ ] **Step 3: Check read-permission on the directory we're about to search (both starting points)**

  Change (around line 2920-2940):
  ```c
          if (isAbs) {               
                  err= RootLSN( pid, dev, spP, false );
              if (err==E_DIDC) err= 0;       /* changes recognized */
              if (err) break;       /* for all other errors: break */
              
              err= ReadFD( spP ); if (err) break;
              if (root) { 
                  strcpy( spP->name,pathname+1 );
                  err= FD_Segment( spP, &attr,&size,&totsize,&sect,&slim, &pref ); if (err) break;
                  rbf->lastPos= size;                   /* last pos is the filesize */
                  rbf->att    = attr;                   /* save attributes */
                  if (!has_perm( pid, attr, FDOwn(spP), permRead )) return E_FNA;
                  return 0; 
              } // if
                                p++; /* cut root path */
              err= CutOS9Path( &p, (char*)&cmp_entry ); if (err) break;
          }
          else {  /* if string is empty */
              if (*pathname==NUL) { err= E_FNA; break; }
              rbf->fd_nr= ls; /* take current path */
              err= ReadFD( spP );      if (err) break;
          }
  ```
  to:
  ```c
          if (isAbs) {               
                  err= RootLSN( pid, dev, spP, false );
              if (err==E_DIDC) err= 0;       /* changes recognized */
              if (err) break;       /* for all other errors: break */
              
              err= ReadFD( spP ); if (err) break;
              if (root) { 
                  strcpy( spP->name,pathname+1 );
                  err= FD_Segment( spP, &attr,&size,&totsize,&sect,&slim, &pref ); if (err) break;
                  rbf->lastPos= size;                   /* last pos is the filesize */
                  rbf->att    = attr;                   /* save attributes */
                  if (!has_perm( pid, attr, FDOwn(spP), permRead )) return E_FNA;
                  return 0; 
              } // if
              if (!has_perm( pid, FDAtt(spP), FDOwn(spP), permRead )) { err= E_FNA; break; }
                                p++; /* cut root path */
              err= CutOS9Path( &p, (char*)&cmp_entry ); if (err) break;
          }
          else {  /* if string is empty */
              if (*pathname==NUL) { err= E_FNA; break; }
              rbf->fd_nr= ls; /* take current path */
              err= ReadFD( spP );      if (err) break;
              if (!has_perm( pid, FDAtt(spP), FDOwn(spP), permRead )) { err= E_FNA; break; }
          }
  ```

- [ ] **Step 4: Check read-permission on every directory descended into, and require write on the parent when creating**

  Change (around line 2950-2963):
  ```c
      while (true) {               dir_len= DIRENTRYSZ; /* read 1 dir entry */
              err= DoAccess( spP, &dir_len, (char*)&dir_entry, false,false ); 
          if (err) {
              if (err==E_EOF) {           /* do not create new sub paths !! */
                  if (cre && strcmp( p,"" )==0) {            /* create it ? */
                      err= CreateNewFile( pid, spP, procs[pid].fileAtt,
                            (char*)&cmp_entry, procs[pid].cre_initsize );
                      rbf->currPos= 0;  /* initialize position to 0 */
                      rbf->lastPos= 0;
                  }
                  else err= E_PNNF; /* OS-9 expects E_PNNF, if entry not found */

              }
              break; /* leave loop also, if file has been correctly created */
          } /* if */
  ```
  to:
  ```c
      while (true) {               dir_len= DIRENTRYSZ; /* read 1 dir entry */
              err= DoAccess( spP, &dir_len, (char*)&dir_entry, false,false ); 
          if (err) {
              if (err==E_EOF) {           /* do not create new sub paths !! */
                  if (cre && strcmp( p,"" )==0) {            /* create it ? */
                      if (!has_perm( pid, FDAtt(spP), FDOwn(spP), permWrite )) {
                          err= E_FNA;
                      }
                      else {
                          err= CreateNewFile( pid, spP, procs[pid].fileAtt,
                                (char*)&cmp_entry, procs[pid].cre_initsize );
                          rbf->currPos= 0;  /* initialize position to 0 */
                          rbf->lastPos= 0;
                      }
                  }
                  else err= E_PNNF; /* OS-9 expects E_PNNF, if entry not found */

              }
              break; /* leave loop also, if file has been correctly created */
          } /* if */
  ```

  Then, still inside the loop, change (around line 2975-2984):
  ```c
                               rbf->fd_nr= DirLSN( &dir_entry );
              err= ReadFD    ( spP );                                          if (err) break;
              err= FD_Segment( spP, &attr,&size,&totsize,&sect,&slim, &pref ); if (err) break;
              rbf->lastPos= size;                   /* last pos is the filesize */
              rbf->att    = attr;                   /* save attributes */
              isFileEntry= (attr & 0x80)==0x00;     /* recognized as file entry */
              
              err= CutOS9Path( &p, (char*)&cmp_entry ); if (err) break;
  ```
  to:
  ```c
                               rbf->fd_nr= DirLSN( &dir_entry );
              err= ReadFD    ( spP );                                          if (err) break;
              err= FD_Segment( spP, &attr,&size,&totsize,&sect,&slim, &pref ); if (err) break;
              rbf->lastPos= size;                   /* last pos is the filesize */
              rbf->att    = attr;                   /* save attributes */
              isFileEntry= (attr & 0x80)==0x00;     /* recognized as file entry */

              /* every directory reached along the path -- intermediate or the
               * final target -- needs read permission to be searched/entered */
              if (!isFileEntry && !has_perm( pid, attr, FDOwn(spP), permRead )) {
                  err= E_FNA; break;
              }
              
              err= CutOS9Path( &p, (char*)&cmp_entry ); if (err) break;
  ```

- [ ] **Step 5: Check the final file target against the caller's actually-requested access mode**

  Change (around line 2986-2994):
  ```c
                  if   (isFileEntry) {              /* if it is a file entry */
                    if (isFile) {
                      if (cre)  err= E_CEF;         /* already there */
                      else      err= 0;             /* is there as file -> ok */
                    }
  ```
  to:
  ```c
                  if   (isFileEntry) {              /* if it is a file entry */
                    if (isFile) {
                      if (cre)  err= E_CEF;         /* already there */
                      else if (!has_open_perm( pid, attr, FDOwn(spP), *modeP ))
                                err= E_FNA;          /* requested access not granted */
                      else      err= 0;             /* is there as file -> ok */
                    }
  ```

- [ ] **Step 6: `pRdelete` — require write permission on the parent directory**

  Change (around line 3167-3174):
  ```c
          err= usrpath_open( pid,&path, fRBF, pathname,*modeP ); 
      if (err) return err;
          spP= get_syspath ( pid, procs[ pid ].usrpaths[ path ] ); /* get spP for fd sects */
      if (spP==NULL) return os9error(E_BPNUM);
      
      dev= &rbfdev[spP->u.rbf.devnr]; /* can't be assigned earlier */
      dfd=         spP->u.rbf.fddir;
  ```
  to:
  ```c
          err= usrpath_open( pid,&path, fRBF, pathname,*modeP ); 
      if (err) return err;
          spP= get_syspath ( pid, procs[ pid ].usrpaths[ path ] ); /* get spP for fd sects */
      if (spP==NULL) return os9error(E_BPNUM);
      
      dev= &rbfdev[spP->u.rbf.devnr]; /* can't be assigned earlier */
      dfd=         spP->u.rbf.fddir;

      { /* deleting removes an entry from the parent directory: needs write there */
          ushort  dsp;
          os9err  dperr= OpenDir( dev, dfd, &dsp );
          if (dperr) { usrpath_close( pid, path ); return dperr; }
          Boolean okToDel= has_perm( pid, FDAtt(&syspaths[dsp]), FDOwn(&syspaths[dsp]), permWrite );
          os9err  dcerr = CloseDir( dsp );
          if (!okToDel) { usrpath_close( pid, path ); return E_FNA; }
          if (dcerr)    { usrpath_close( pid, path ); return dcerr; }
      }
  ```

- [ ] **Step 7: `pRsetatt` — owner or super-user only**

  Change (line 3362-3367):
  ```c
  os9err pRsetatt( _pid_, syspath_typ* spP, uint32_t *attr )
  /* set the attributes of a file */
  {
      Set_FDAtt     ( spP, (byte)*attr ); /* byte ordering is already correct */
      return WriteFD( spP );
  } /* pRsetatt */
  ```
  to:
  ```c
  os9err pRsetatt( _pid_, syspath_typ* spP, uint32_t *attr )
  /* set the attributes of a file -- owner or super-user only */
  {
      if (!is_super(pid) && !IsOwner(pid, FDOwn(spP))) return E_FNA;
      Set_FDAtt     ( spP, (byte)*attr ); /* byte ordering is already correct */
      return WriteFD( spP );
  } /* pRsetatt */
  ```

- [ ] **Step 8: Build clean**

  Run: `make`
  Expected: no new warnings or errors.

- [ ] **Step 9: Run the Task 2 permission tests — confirm they now all pass**

  Run: `swift run --package-path test "fs: permission"`
  Expected: every test in this group passes now, including the ones that were expected to fail in Task 2.

- [ ] **Step 10: Run the full suite — confirm zero regressions elsewhere**

  Run: `swift run --package-path test`
  Expected: same pass count as Task 1's baseline, plus the Task 2 permission tests now passing. Nothing else changes, because every other test runs as super-user and `is_super()` bypasses all of this.

- [ ] **Step 11: Commit**

  ```bash
  git add Source/OS9exec_core/file_rbf.c
  git commit -m "Fix: enforce RBF owner/public permission bits on open/create/delete/setattr"
  ```

---

### Task 4: Close out the ROADMAP item

**Files:**
- Modify: `ROADMAP.md`

**Interfaces:** none — documentation only.

- [ ] **Step 1: Delete the completed bullet**

  Remove the "**File permissions and ownership don't work — worth fixing, and tractable.**" bullet and its body (the whole item, from `- **File permissions...` through the paragraph ending "...as the guide.") from the "Bugs to run down" section of `ROADMAP.md`. Per the file's own header instruction: "When an item is done, delete it — don't leave it here marked ~~FIXED~~."

- [ ] **Step 2: Commit**

  ```bash
  git add ROADMAP.md
  git commit -m "Docs: remove completed RBF permission-enforcement item from ROADMAP"
  ```

---

## Verification (end-to-end, beyond the automated suite)

After Task 3, do one live sanity pass with `tools/os9repl.sh` against the default `h0` disk (per the `os9-dev` skill's REPL guidance — one command at a time, read each result):

1. `mount -k=200K h8`, `chd /h8`, `login claude`, `chd /h8`, `echo hi >f`, `dump f` — confirm it reads back.
2. `attr f -nr`, `dump f` — confirm `Error #000:214` (E$FNA) this time, not silent success.
3. `logout` back to `su`, `dump f` — confirm the super-user still reads it fine.

This exercises the exact same paths as the automated suite but lets you see OS-9's real error text, not just "contains Error #" — useful to confirm the error code is genuinely `E$FNA` and not some other error string that happens to contain the substring.
