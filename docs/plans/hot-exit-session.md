# Plan: Session Persistence for Unsaved Tabs (Hot Exit)

## Objective

Preserve the state of all open tabs (including unsaved "dirty" content and "Untitled" tabs) when the application is closed, and restore them when the application is reopened.

## Analysis of the Request

The user noted that currently, clicking "Don't Save" on the unsaved changes prompt during app exit results in the tab being lost on the next run.

While we could change "Don't Save" to mean "keep in session but don't save to disk", the standard UX for this (found in editors like VS Code and Sublime Text) is to **not prompt at all on app exit**.

### Proposed UX (Recommended)

1. **Closing a Single Tab:** If the tab has unsaved changes, the app **will prompt** you ("Save", "Don't Save", "Cancel"). "Don't Save" will completely discard the tab.
2. **Closing the App:** The app **will NOT prompt** you. It will instantly close. All tabs, including their unsaved edits and "Untitled" tabs, are saved in a temporary session backup.
3. **Reopening the App:** The app loads the session backups. All tabs are restored exactly as you left them, with their "unsaved" (`*`) status intact.

## Implementation Steps

- [x] **1. Extend `SessionManager` format**
  - Instead of just saving a list of file paths, the `session.txt` will store properties for each tab.
  - Format per tab (3 lines each):
    ```text
    <FilePath (or empty if Untitled)>
    <IsDirty (1 or 0)>
    <BackupFileName (e.g., backup_0.txt, or empty if not dirty)>
    ```
  - When saving the session, `SessionManager` will iterate through all tabs. If a tab is dirty (or untitled), it will write the current text of the editor to a `backup_X.txt` file in the configuration directory.

- [x] **2. Modify `NotepadApp::Run` (App Exit Logic)**
  - Change the `m_state.triggerExit` logic. When exiting the app, we bypass the `showUnsavedChangesModal` check entirely. We just let the loop end `running = false`.
  - The `SessionManager::SaveSessionState` function will take a struct of tab details and the actual editor contents instead of just file paths.

- [x] **3. Modify `NotepadApp::Init` (Startup Logic)**
  - `SessionManager::LoadSessionState` will return the list of tabs and their backup paths.
  - For each tab, if it has a backup path, the app will read the backup file and inject the text into the editor, while marking it as `isDirty = true`.
  - If it's not dirty, it just loads the original file path asynchronously like it does now.

- [x] **4. Clean up backups**
  - When saving a new session, old backup files from the previous session in the config directory should be deleted to prevent the directory from growing indefinitely.

- [x] **5. Unit Tests updates**
  - Add Catch2 tests for `SessionManager` to verify it correctly serializes and deserializes the new `session.txt` format, including the dirty state and backup file creation.
