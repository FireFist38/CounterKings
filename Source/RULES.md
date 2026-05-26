DESIGN RULES:

## CRITICAL MANDATE: FILE INTEGRITY & NO TRUNCATION
AS AN AI AGENT, YOU ARE STRICTLY PROHIBITED FROM TRUNCATING ANY FILE CONTENT. YOU MUST READ FILES IN THEIR ENTIRETY. IF A FILE IS TOO LARGE, YOU MUST READ IT IN CHUNKS. NEVER ATTEMPT TO EDIT, REPLACE, OR MODIFY A FILE WITHOUT HAVING THE FULL, NON-TRUNCATED CONTENT IN YOUR CONTEXT. IF YOU ARE UNSURE, ASK.

Everything is to be done in C++, functions, variables, UI bindings, EVERYTHING. There will be little to no BP implementation, only when NECESSARY.
As an AI coding agent, you are to not be lazy and leave work to be done via BPs, you are to create the entirety of a system or function and tell me the necessary editor steps (ex: reparenting, setting values, dropdowns, binding / variable names, etc..)

EVERYTHING you do needs to be NETWORK REPLICATED, when applicable. This is an online game, exclusively.

EVERYTHING you do needs to be BP exposed, editable, when applicable. Never hardcode values without giving me the ability to tweak them in editor.

We will be using ENHANCED Inputs, NOT legacy.
FOR THE LOVE OF GOD, YOU ARE AI, YOU SHOULD NOT BE GENERATING SYNTAX ERRORS, EVER.
Strict UE5 API Adherence: "Never guess a function signature. If unsure of an actor's method, ask me to provide the header or tell me to check the docs.

Macro Integrity: Always include necessary Unreal macros: UFUNCTION(), UPROPERTY(), and GENERATED_BODY(). Never omit the .generated.h include. If a function is meant for Blueprints, explicitly include BlueprintCallable or BlueprintNativeEvent.
No 'Shadow' Variables: Do not invent member variables that aren't in the provided header, unless its necessary for a brand new function or system you are implementing. NEVER 'guess' variable names if you are unsure.
The 'Include' Rule: Do not assume standard C++ headers work. Always prioritize Unreal's CoreMinimal.h and specific module headers
Performance Awareness: Avoid heavy logic in Tick(). Prefer Timers or Events. For distance checks, use FVector::DistSquared instead of Dist to save on square root calculations.

We will prioritize using Enums / data tables when necessary.

Stats, inventory, abilities, etc.. should be components on the CHARACTER, not the controller, or anywhere else.

If I ask a question you are to answer it, NOT start implementing things without asking.
You are to NEVER call anything just 'slot', that's an immediate compile error.

The Circuit Breaker: If I say 'RE-ALIGNE', you must stop what you are doing, re-read this entire Rules.md, and summarize the current task's architecture in 3 bullet points before providing any more code. This is to prevent error cascading.

This Project is a template of "GASP", (Game Animation Sample Project), thus potential conflicts in functionality, primarily animation related things, can often times be overridden by GASPS robust BP logic. In order to make things work, we use GASPS "Update_PropertiesFromCharacter" Function in their Animation BP to incorporate new logic / and set variables / character states, that will allow our functionality to co-exist and be driven by GASPS animation without clashing with it.

STRICT EDITING PROTOCOL:
- ABSOLUTELY NO 'REPLACE' TOOL: The 'replace' tool is strictly prohibited. It creates too many opportunities for error via malformed search strings or accidental deletions.
- MANDATORY READ-BEFORE-WRITE: You MUST read the full content of a file using 'read_file' before writing any changes.
- COMPLETE FILE OVERWRITES: All code changes must be performed using 'write_file' by writing the complete, updated content of the file. No partial edits, no snippets.
- PRE-WRITE VALIDATION: After reading a file, you must internally review the state of the entire file to ensure your proposed changes are syntactically and contextually complete before applying the 'write_file' operation.

ERROR LOGGING & ACCOUNTABILITY:
- Mandatory Annotation: Every build error must be annotated in this file below to ensure systemic prevention.
- Tracking Categories:
    - Include Management: Failure to correctly map and include header dependencies.
    - Access Control: Incorrectly setting access modifiers (public/protected/private) for inter-class communication.
    - Member Lifecycle Management: Accidentally deleting or omitting required member variables during file overwrites.
    - API Signature Mismatch: Implementing or calling functions without verifying the required parameters or return types.
    - Const Correctness: Failing to mark getter methods as 'const', preventing their use in const-correct contexts like the HUD widget.
    - File Truncation: Failure to read or write a file in its entirety, leading to loss of logic.
