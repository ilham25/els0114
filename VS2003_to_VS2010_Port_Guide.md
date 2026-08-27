# Porting a VS2003 (VC7.1) C++ project to VS2010 (VC10)

Written from porting the five ProjectX2/Elsword servers (`KncWX2Server/{GameServer,LoginServer,ChannelServer,CenterServer,GlobalServer}`) from VS2003-only to also building under VS2010, on 2026-08-27. The servers hadn't been touched by anything newer than VC7.1 in over a decade. Everything below generalizes past this specific codebase — the section headers are the reusable process; the indented examples are what it looked like here.

**Read this first if:** you have an old MSVC codebase (VC6/VC7.1/VC8) that only builds with a toolchain you can no longer install or don't want to depend on, and you need it building under a modern Visual Studio (2010+) without a rewrite.

---

## 0. Before you start: decide the shape of the port

Three decisions up front save real rework later.

**Keep the old project files, or convert in place?** Keep them. Copy `.vcproj` → `.vcxproj`, don't edit `.vcproj` in place and hope the IDE upgrades it losslessly. You want a working fallback and a diff-able "what changed" story. In this port, `X2Project_2003.sln` and every `*_2003.vcproj` were left byte-for-byte untouched; only new `_2010.vcxproj`/`.sln` files were added.

**New solution, or merge into an existing one?** If the target solution is small and single-purpose, merge is fine. If it's large and already serves a different toolchain/config (a client build, in this case), keep the ported projects in a **separate solution file** instead of adding new solution-configurations to the existing one. Reasons:
- Adding a config to a big solution means deciding, for *every* project in it, whether it builds under the new config or gets skipped — a lot of surface area to get wrong in a file you don't want to break.
- If the ported projects are *already* registered in the big solution under a stale/dead config (common when someone attempted this port years ago and gave up), adding a second, working config for the same projects in the same file creates a "two ways to build this, one is dead" trap.

  Here: `X2Project_Servers_2010.sln`, new, at the repo root, containing just the five (six, see §7) ported projects with a single `Release_US|Win32` configuration — sitting next to the pre-existing `X2Project_2010.sln` (client-only, unrelated) and the untouched `X2Project_2003.sln`.

**What's your ground truth?** Before changing anything, find a **successful build log from the old toolchain** for the exact configuration you're porting. It tells you, unambiguously: the exact compiler/linker command line (including flags the old project file doesn't show because they were *toolchain defaults*, not explicit settings — this bit us, see §4), and the exact set of files that got compiled for that specific configuration (project files almost always list more files than any one configuration actually builds — some are commented out, `#ifdef`'d away, or `ExcludedFromBuild` per-config).

  Here: `KncWX2Server/<Server>/Release_US/BuildLog.htm`, still sitting in-tree from the last real build. Its command line and file list were the reference every later step got checked against.

---

## 1. Convert the project files

Don't hand-write the new project file and don't trust a from-scratch IDE "Add Existing Project" + manual settings pass — you will silently drop files or settings. Use the vendor's own converter, then edit the result.

1. Copy `<Project>_<old>.vcproj` to a new name in the same directory (e.g. `<Project>_2010.vcproj`) — the converter names its output after the input, so you can't point it straight at the final `_2010.vcxproj` name without an intermediate rename.
2. Run the converter:
   ```
   "<VS install>\Common7\Tools\vcupgrade.exe" /overwrite <Project>_2010.vcproj
   ```
   `/overwrite` matters — without it, a second run refuses to touch an existing output. `vcupgrade` runs from the command line, no IDE session needed.
3. It prints a manifest of every semantic change it made — **read it**, don't just check the exit code. In this port it flagged, correctly, that some source used the "bare member-function-pointer" VC7.1 extension that VC10 rejects (see §4) — a genuine heads-up about upcoming compile errors, not noise.
4. Rename the input away (delete the scratch `.vcproj` copy) and rename the outputs (`.vcxproj`, `.vcxproj.filters`) into place.
5. **Verify nothing was lost**, mechanically, not by eyeballing:
   - File count: `grep -c '<ClCompile Include=' new.vcxproj` should equal the old project's compiled-file count (careful: old-format project files often mix `.cpp` and `.c` under one `RelativePath=` pattern — count both).
   - GUID: `<ProjectGuid>` in the new file must match `ProjectGUID=` in the old one. If the project is already referenced by another solution you're keeping (or by a solution you'll later re-target), keeping the GUID means you don't have to touch that other file at all.

Do this once per project, then move on — don't try to hand-fix settings during the conversion pass. That's the next phase.

---

## 2. Trim to the one configuration you actually need

Old multi-region/multi-environment codebases accumulate dozens of solution configurations. `vcupgrade` carries all of them over. If you only need one (or a handful), delete the rest now, mechanically — hand-editing 20+ configs across every project file is exactly the kind of work you should script.

The pattern to strip, in every `.vcxproj`:
- `<ItemGroup Label="ProjectConfigurations">` — delete every `<ProjectConfiguration>` except the ones you're keeping.
- Any element — `<PropertyGroup Label="Configuration">`, `<ImportGroup Label="PropertySheets">`, `<ItemDefinitionGroup>`, or a **leaf** property like `<OutDir Condition="...">` or `<ExcludedFromBuild Condition="...">` — whose `Condition` attribute references a configuration you're dropping. Group-level elements (`PropertyGroup`, `ImportGroup`, `ItemDefinitionGroup`) get deleted whole; leaf elements just get that one conditioned instance deleted, leaving their siblings for the kept configs untouched.

A small script (regex on the `Condition` attribute against your keep-list) handles this safely across every project in a batch — see the working version at the bottom of this doc. Two things it must get right:
- Preserve per-file `ExcludedFromBuild` entries for the configuration you *kept* — those aren't clutter, they're load-bearing (a file legitimately excluded from your target config must stay excluded).
- Use `XmlWriter` with `Indent = true`, not `PreserveWhitespace = true` — the latter leaves ugly (but harmless) blank-line gaps where deleted siblings used to be. Cosmetic, but worth getting right once.

Verify again after stripping: same file count, exactly one `<ProjectConfiguration>` left, still parses.

---

## 3. Extract shared build settings

Old project files often rely on **toolchain defaults that changed** between old and new MSVC — not on anything explicit in the file. The project file won't tell you this; the build log from §0 will, because it shows what the compiler was *actually* invoked with.

Concretely, compare your old build log's command line against what the new project's default resolves to:
- **`Optimization`** — VC7.1 defaulted release builds to something roughly equivalent to `/O2` even when the project file set nothing. MSBuild's default is `Disabled`. If the old `.vcproj` doesn't have an explicit `<Optimization>` and your build log shows `/O2`, you must add it explicitly or you'll ship an unoptimized binary and never notice from the build succeeding.
- **`ExceptionHandling`** — same story; VC7.1 defaulted to `/EHsc`-equivalent, MSBuild defaults to none.
- Any macro whose *width* changed between compilers (the classic one: `time_t` was 32-bit by default pre-VC8, 64-bit from VC8 on — if the old project didn't set `_USE_32BIT_TIME_T` because it didn't need to, and your data format (wire protocol, on-disk records, a shared client that's still on the old toolchain) assumes the old width, you need to define it explicitly now or every `time_t` field silently changes size across a boundary that used to agree by accident).
- CRT deprecation warnings (`_CRT_SECURE_NO_WARNINGS` etc.) — the new compiler will flag hundreds of calls (`sprintf`, `strcpy`, ...) the old one didn't. If fixing all of them isn't the goal of this pass, suppress the warning family at the project level rather than touching every call site.
- Any preprocessor-directive style the codebase already leans on that the new compiler is stricter about (see the `#endif TOKEN` note in §4) — same idea: suppress the specific warning rather than mass-editing the source, if the house style is intentional and pervasive.

Put all of this in one shared `.props` file, imported by every ported project's `PropertySheets` import group, rather than duplicating it per-project. Only per-project settings (its own include/lib paths, its `ConfigurationType`) stay in the individual `.vcxproj`.

**Include/library paths that point at the old toolchain's SDK need retargeting**, file-by-file across each project (this doesn't factor into the shared `.props` cleanly, since the strings themselves differ per dependency): swap any path that names the old compiler version (`vc71`, `vc9`, a versioned SDK folder) for whatever the new toolchain's equivalent is. Check whether your vendored/in-house SDKs already *have* a newer-toolchain build sitting in the tree unused — old projects that were ported once before and abandoned often left one behind.

  Here: the client side of this same codebase had already been ported to VS2010 years earlier and left `KNCSDK/Include_2010/` + `KNCSDK/lib_2010/` in the tree, unused by anything server-side. Pointing the server projects at those instead of `Include`/`lib` did most of the retargeting for free — but see §5, that snapshot turned out to be incomplete.

---

## 4. Fix compile errors

Once configs and settings are sorted, most remaining compile errors fall into a short list of **recurring bug classes** — the old compiler was lenient about each of these in a specific, nameable way, and the new one correctly rejects it. Once you've fixed one instance you can grep for the rest.

**Bare `Class::Method` as a member-function pointer.** `SomeFunc( obj, Class::Method, ... )` where the parameter type is `RetType (Class::*)(Args)` — legal as a non-standard extension pre-VC8, a hard error after. Fix: add the `&`. `vcupgrade` itself will warn you this class of break exists in the codebase before you even attempt a build (see §1) — grep for the exact call site pattern once you know it (e.g. `new SomeTemplate<T>( *this, T::MethodName,` in this codebase) and fix every instance in one pass rather than one-by-one as the compiler finds them.

**Macros that were never actually defined, anywhere, for the files that call them.** Sounds impossible — it isn't. Old compilers with lenient "implicit int" / K&R-style leftover leniency will parse an unknown identifier followed by `(args);` as an implicit-int function *declaration* (a no-op prototype, never called) rather than an error. If a macro like `IMPL_SOME_FRAMEWORK_THING( ClassName );` appears at file scope, and grepping the whole tree turns up **no** `#define` for it, you've found dead ceremonial boilerplate that only "worked" by accident. The new compiler correctly reports this as C4430 ("missing type specifier — int assumed"). Fix: comment out the call (check first whether sibling files already do this — in a large codebase, previous maintainers often already found and commented out most instances, leaving only a few strays for you to catch up on).

**A loop variable used after its loop closes.** `for( int i = 0; ...; ++i ) {...} /* later */ use(i);` — legal under the old compiler's non-conformant default for `for`-scope, an "undeclared identifier" after. Fix: hoist the declaration above the loop, drop the `int` from the loop header. Trivial once you see it, easy to miss reading fast because it looks completely ordinary.

**A container iterator dereferenced into a non-`const` parameter.** Old STL implementations sometimes handed out a mutable reference from `std::set<T>::iterator::operator*()` (technically non-conformant — a `set`'s iterator is supposed to be `const`-only, since mutating an element could break the tree's ordering invariant). New STL correctly returns `const T&`. If that value then gets passed to something expecting `T&`, you get a conversion error where none existed before. Fix: copy into a local non-`const` variable first, rather than changing the callee's signature (safer — you don't know every other caller's assumptions).

**Missing standard-library includes that used to arrive transitively.** `<set>`, `<vector>`, `<iterator>`, etc. — an old, deep standard-library header graph pulled these in as a side effect; the new one doesn't. Fix: add the explicit `#include`.

**The project's own historical preprocessor convention triggering a new warning-as-noise.** If the codebase has a house style like `#endif SOME_TOKEN` (a bare identifier after `#endif`, technically invalid standard C++ but universally MSVC-tolerated) used *thousands* of times, don't mass-rewrite it — that's enormous unrelated churn for a warning, and it's not actually wrong. Suppress the specific warning number at the project level (see §3) instead.

**A vendored/in-house header that's missing a function or type your code calls unconditionally.** This happens when a "newer toolchain" version of an internal SDK header exists in the tree (see §3's tip about finding one already left behind) but is an **older, incomplete snapshot** relative to the version actually being compiled against on the old toolchain — someone forked it once and never kept it in sync. The compiler error looks identical to a normal missing-declaration problem (`'X' is not a member of 'Y'`), but the fix is different: **diff the old header against the new one**, find the missing function/type, and port it forward — don't stub it out, and don't assume it's dead code just because it's missing from the newer copy. Grep every call site first to confirm it's genuinely called unconditionally (not gated behind a flag that's off for your target config) before spending time on the backport.

**A template-based binding/reflection library (Lua bindings, RPC dispatch, serialization — anything built on variadic-by-hand C++03 template families) capped at a lower argument count than a real call site needs.** C++03-era codebases often implement "variadic" support as a manually-repeated template family — `Func<T1>`, `Func<T1,T2>`, `Func<T1,T2,T3>`, ... up to some N the original author decided was enough. If a newer part of the codebase grew a function with more parameters than N, and the version of the library that made it into your "newer toolchain" snapshot is the *older*, lower-N one, you'll get overload-resolution errors that look like "too many/few arguments" for a template, not a normal missing-symbol error. Fix: extend the template family by copy-pasting the existing pattern one arity higher — but check whether extending the **primary** template (if it has defaulted type parameters) also requires adding an **explicit specialization for the arity the primary used to represent alone**, or the arity right below your new max will silently break instead (partial specialization + a growing set of default template args interact in a way that's easy to get backwards — verify by rebuilding every dependent arity, not just the new one, before moving on).

---

## 5. Fix link errors

Two distinct genuinely different classes here.

**A dependency is only shipped as a prebuilt static library for the old toolchain, and you have no source.** First check: is it a real static library (has its own code, own CRT linkage) or a thin **import library** for a DLL (just linkage stubs, the actual code lives in the `.dll` and gets loaded at runtime)? Check with your toolchain's object-dump tool for the library's declared default-library dependencies — a thin import lib for a DLL will link against practically anything since it does no real work at link time; a fat static lib brings its own CRT assumptions along and can genuinely conflict. In this port, five prebuilt libraries turned out to be thin import libs (fine, linked with zero changes) and only one was a real static lib (also fine, once checked — its declared CRT dependency was version-agnostic). If a static lib truly conflicts, your only options are: get it rebuilt from source for the new toolchain, or drop the feature it provides for this build.

**A prebuilt binary for the new toolchain exists in the tree, but no project file to reproduce it exists anymore.** This happens to internal SDKs that were "ported once, by hand, off-tree" years before — someone built the new-toolchain `.lib` on their own machine, checked in the binary, and never checked in the project that made it. If you need to change that library's source later (see §4's backport case), you're stuck rebuilding a `.lib` with no way to do it — **until you need to, at which point you must reconstruct a minimal project file for it**: convert its own old project file the same way as §1, strip to the one runtime-library variant you actually need (check what every consumer links against — usually just `/MD` release, sometimes also `/MDd` debug; drop variants like `/ML` that the new toolchain's `RuntimeLibrary` setting doesn't even have distinct values for anymore), retarget its own include paths at the newer SDK snapshot, and build it standalone with a direct `msbuild <that>.vcxproj /p:Configuration=<config>` — it doesn't need to join any solution.

**Two source files, from different parts of the tree, share the same basename and export the same global symbols — and the object-file collision was previously hiding a real bug.** This is subtle and worth its own callout. Old single-threaded, sequential compilers wrote each `.cpp`'s output to a default object-file name derived purely from the basename (not the full relative path); if two files in the project share a basename (`stdsoap2.c` under two different vendored-service-integration folders is a classic real-world example — generated SOAP/RPC client code for two different regions/services, same filenames, same generated symbol names), the old build silently overwrote one `.obj` with the other before the linker ever saw both — meaning **one of the two implementations was never actually linked into the shipped binary**, the whole time, with no error, because the collision hid it. The new toolchain, especially once you turn on multi-processor compilation (`/MP` / `MultiProcessorCompilation`), makes this a **hard, loud failure**: either a file-locking error (two compiler processes fighting over the same output path) or, once you naively give each file a unique object name to fix that, a **duplicate-symbol link error** (now the linker genuinely sees both). Don't just paper over the duplicate-symbol error by re-colliding the object names again — that restores the *original* silent bug. Instead, work out which of the two implementations is actually needed for the configuration you're building (check whether the code that calls into it is itself dead/`#ifdef`'d out for your target — it very often is, since this pattern shows up most in per-region/per-service code where only one region ships per build), and **exclude the other from that configuration's compile list**, same as you'd exclude any file that's simply irrelevant to this config. Check whether a *sibling* project in the same codebase already had the exclusion done correctly (a good sign someone hit this before you and fixed it in one place but not everywhere) — if so, match that pattern exactly rather than re-deriving it.

---

## 6. Runtime dependencies

The build succeeding doesn't mean the binary can run yet on the box that built it, let alone anywhere else.

1. **List actual DLL dependencies** with your toolchain's dependency-walker equivalent (`dumpbin /DEPENDENTS <exe>` for MSVC). Do this per-binary if you have several — they may not all need the same set.
2. **New-toolchain CRT/framework runtime** (e.g. `msvcr100.dll`, `msvcp100.dll`, an MFC runtime DLL if the project links it dynamically) — source these from the new IDE's own `redist` folder (`<VS install>\VC\redist\<arch>\Microsoft.VC*.CRT\`, `...\Microsoft.VC*.MFC\`), not from the internet. Copy alongside the binary rather than requiring a system-wide redistributable install if you want the binary to be self-contained/portable.
3. **In-tree custom DLLs.** Old deployments often had these DLLs sitting loose in some directory that a manual/scripted copy step put next to the exe at deploy time — not the build's own output directory. Search the whole tree for each DLL named in the dependents list; check every directory it turns up in for a timestamp/version that looks canonical (a "last deployed region" folder is a strong hint you're looking at the right copy).
4. **Anything the new build retargeted to a different vendor-toolchain variant** (per §3's SDK path retargeting) needs its **matching runtime DLL**, not just its `.lib` — if you pointed the linker at, say, a different compiler-version folder of a vendored library's import libs, the matching `.dll` lives in that same vendor folder, not the one the old build used.
5. Verify by actually diffing the dependents list against what's sitting in your deployment directory — don't assume "it built" implies "it'll run."

---

## 7. If it builds and launches but crashes instantly: check for self-modifying code and DEP

This is specific enough to be worth its own section rather than folding into §4/§5, because it looks nothing like a normal bug and the standard debugging instinct (read the stack trace, blame the last thing you changed) leads nowhere useful — the crash is often in completely unmodified, unrelated-looking code.

**Symptom:** the exe builds clean, every DLL dependency is satisfied, config/DB files are all in place — and it still dies immediately with an access violation (`0xC0000005` on Windows), often inside a Windows API call like `SendMessage`, with a faulting address that doesn't resolve to any symbol in your binary at all (it resolves to a raw stack or heap address instead of anything in your code's address range).

**Likely cause:** somewhere in the codebase — often in old UI/windowing helper code nobody's touched in a decade — there's a hand-rolled "thunk": code that writes raw machine-code bytes into an ordinary data structure at runtime (a `mov`/`jmp` sequence baked in byte-by-byte, usually to fake injecting a `this` pointer into a plain C callback), then has some other code (often a native API, like a window procedure callback) jump into that memory as if it were a function. This was a completely normal, working technique when DEP (Data Execution Prevention) either didn't exist or defaulted to "opt-in, applies only to OS binaries." On any modern Windows install, DEP is enforced by default on essentially everything, and executing unmarked (non-executable-flagged) memory is an instant hard fault — regardless of which compiler built the code. **This means the exact same crash would happen with the original old-toolchain binary too, if you ran it on this same modern machine** — it's not something the port introduced, it's a decade-old landmine that a modern OS finally steps on.

**How to confirm it's this, cheaply:** find where in the code the raw bytes get written — grep for something like `FlushInstructionCache` (a very strong signal: you only need that API when you've just written code into memory you're about to execute) or for hand-encoded opcode constants being assigned to `DWORD`/`BYTE` struct members. If you find one, this is almost certainly your answer without even needing a debugger.

**Fix:** call `VirtualProtect(&thatStruct, sizeof(thatStruct), PAGE_EXECUTE_READWRITE, &oldProtect)` right where the bytes get written, before anything jumps into it. This is the standard, well-precedented fix for this exact pattern (a widely-used ATL helper needed the identical patch historically for the same reason) — it doesn't change behavior on any OS where DEP was already lax, and it's what lets the technique keep working on a modern one.

### Diagnosing a crash like this with no debugger installed

If you don't have `windbg`/`cdb` available and only have the IDE itself, you can still get a symbolized stack trace without attaching a live debugger session:

1. Find the earliest-executing piece of your own code — ideally a global object's constructor, since C++ static initialization runs before your actual entry point's body does. Add a temporary `SetUnhandledExceptionFilter` call there.
2. The filter function: open a log file, write `ExceptionRecord->ExceptionCode` and `ExceptionAddress`, then use `dbghelp.h`'s `SymInitialize` + `SymFromAddr` + `SymGetLineFromAddr64` to resolve that address against your `.pdb`, and a manual `StackWalk64` loop (seeded from the exception's `CONTEXT`) to resolve a handful of calling frames the same way.
3. Rebuild with this instrumentation, **copy the matching `.pdb` alongside wherever you're actually running the crashing binary** (symbol resolution needs it sitting next to the exe, or on an explicit search path), reproduce the crash, read the log.
4. Once you've found and fixed the real bug, **remove the temporary instrumentation** and do one final clean rebuild + retest before calling it done — don't ship the diagnostic code.

This is slower than a real debugger but entirely scriptable from a terminal, and it's usually enough: you rarely need more than the immediate faulting frame plus two or three callers to recognize the pattern.

---

## Reusable checklist

- [ ] Find a build log from the old toolchain for your exact target config — this is your ground truth for the rest of the process.
- [ ] Copy, don't edit in place, the old project files. Convert copies with the vendor upgrade tool.
- [ ] Verify file count and GUID survived conversion, mechanically.
- [ ] Trim to just the configuration(s) you need, in every converted project.
- [ ] Diff the old build log's actual command line against the new project's defaults — add back anything the old toolchain silently defaulted to that the new one doesn't (optimization, exception handling, integer/time widths, CRT-deprecation and legacy-preprocessor-style warning suppression).
- [ ] Retarget vendored SDK include/lib paths; check for an already-abandoned newer-toolchain snapshot in the tree before assuming you need to build one from scratch.
- [ ] Compile, fix errors by class (see §4's list) rather than one at a time — most instances of a given class can be found and fixed together once you recognize the pattern.
- [ ] Link, fix errors by class (see §5) — watch specifically for same-basename object-file collisions that may have been silently hiding a missing feature.
- [ ] `dumpbin /DEPENDENTS` (or equivalent) every output binary; source every DLL it needs from the new toolchain's redist folder or from wherever the old deployment actually kept it (often not the build output directory).
- [ ] Actually run the binary. If it crashes instantly with no obvious cause, check for hand-rolled executable-data thunks before assuming the port itself is broken.
- [ ] Remove any temporary diagnostic instrumentation before considering the port done.

---

## Appendix: working config-stripping script (PowerShell)

Referenced in §2. Strips a converted `.vcxproj` down to one configuration, handling both group-level and leaf-level `Condition` attributes, with clean re-indented output (no `PreserveWhitespace` gaps).

```powershell
param(
    [Parameter(Mandatory=$true)][string]$Path,
    [string]$Keep = "Release_US|Win32"   # <-- change to your target config
)

$xml = New-Object System.Xml.XmlDocument
$xml.Load($Path)

$nsmgr = New-Object System.Xml.XmlNamespaceManager($xml.NameTable)
$nsmgr.AddNamespace("msb","http://schemas.microsoft.com/developer/msbuild/2003")

# 1. Trim ProjectConfigurations to just the kept one.
$pcGroup = $xml.SelectSingleNode("//msb:ItemGroup[@Label='ProjectConfigurations']", $nsmgr)
$pcNodes = @($pcGroup.SelectNodes("msb:ProjectConfiguration", $nsmgr))
foreach ($n in $pcNodes) {
    if ($n.GetAttribute("Include") -ne $Keep) {
        [void]$pcGroup.RemoveChild($n)
    }
}

# 2. Remove every element (group-level or leaf) whose Condition references
#    Configuration|Platform for a config other than the one we keep.
$condNodes = @($xml.SelectNodes("//*[@Condition]"))
foreach ($n in $condNodes) {
    $cond = $n.GetAttribute("Condition")
    if ($cond -match "'\`$\(Configuration\)\|\`$\(Platform\)'=='([^']+)'") {
        $cfg = $matches[1]
        if ($cfg -ne $Keep) {
            [void]$n.ParentNode.RemoveChild($n)
        }
    }
}

$settings = New-Object System.Xml.XmlWriterSettings
$settings.Indent = $true
$settings.IndentChars = "  "
$settings.Encoding = New-Object System.Text.UTF8Encoding($true)   # keep the BOM
$settings.NewLineChars = "`r`n"

$writer = [System.Xml.XmlWriter]::Create($Path, $settings)
try { $xml.Save($writer) } finally { $writer.Close() }
```

Run once per converted `.vcxproj`, after §1's conversion and before §3's shared-settings pass.
