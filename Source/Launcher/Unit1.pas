unit Unit1;

interface

uses
  Windows, Messages, SysUtils, Variants, Classes, Graphics, Controls, Forms,
  Dialogs, Menus, ExtCtrls, ShellAPI, IniFiles, XPMan, Registry;

type
  TMain = class(TForm)
    TrayPopupMenu: TPopupMenu;
    CloseBtn: TMenuItem;
    N2: TMenuItem;
    N1: TMenuItem;
    RunStopBtn: TMenuItem;
    RunInBgBtn: TMenuItem;
    SetupBtn: TMenuItem;
    N5: TMenuItem;
    ConfigBtn: TMenuItem;
    N6: TMenuItem;
    GamepadTestBtn: TMenuItem;
    ShowHideAppBtn: TMenuItem;
    CheckAppClosedTimer: TTimer;
    XPManifest1: TXPManifest;
    N4: TMenuItem;
    KMProfilesBtn: TMenuItem;
    HidHideSubMenu: TMenuItem;
    N3: TMenuItem;
    UtilitiesBtn: TMenuItem;
    Utility1Btn: TMenuItem;
    Utility2Btn: TMenuItem;
    Utility3Btn: TMenuItem;
    Utility4Btn: TMenuItem;
    Utility5Btn: TMenuItem;
    Utility6Btn: TMenuItem;
    Utility7Btn: TMenuItem;
    Utility8Btn: TMenuItem;
    Utility9Btn: TMenuItem;
    Utility10Btn: TMenuItem;
    AutostartBtn: TMenuItem;
    XboxProfilesBtn: TMenuItem;
    HidHideAddBtn: TMenuItem;
    OpenDialog: TOpenDialog;
    HideHideClearBtn: TMenuItem;
    HideGamepadBtn: TMenuItem;
    HidHideRunBtn: TMenuItem;
    HidHideRemBtn: TMenuItem;
    N9: TMenuItem;
    N10: TMenuItem;
    N7: TMenuItem;
    procedure FormCreate(Sender: TObject);
    procedure CloseBtnClick(Sender: TObject);
    procedure FormClose(Sender: TObject; var Action: TCloseAction);
    procedure RunInBgBtnClick(Sender: TObject);
    procedure RunStopBtnClick(Sender: TObject);
    procedure ConfigBtnClick(Sender: TObject);
    procedure GamepadTestBtnClick(Sender: TObject);
    procedure ShowHideAppBtnClick(Sender: TObject);
    procedure CheckAppClosedTimerTimer(Sender: TObject);
    procedure KMProfilesBtnClick(Sender: TObject);
    procedure Utility1BtnClick(Sender: TObject);
    procedure Utility2BtnClick(Sender: TObject);
    procedure Utility3BtnClick(Sender: TObject);
    procedure Utility4BtnClick(Sender: TObject);
    procedure Utility5BtnClick(Sender: TObject);
    procedure Utility6BtnClick(Sender: TObject);
    procedure Utility7BtnClick(Sender: TObject);
    procedure Utility8BtnClick(Sender: TObject);
    procedure Utility9BtnClick(Sender: TObject);
    procedure Utility10BtnClick(Sender: TObject);
    procedure AutostartBtnClick(Sender: TObject);
    procedure XboxProfilesBtnClick(Sender: TObject);
    procedure HidHideAddBtnClick(Sender: TObject);
    procedure HideHideClearBtnClick(Sender: TObject);
    procedure HideGamepadBtnClick(Sender: TObject);
    procedure HidHideRemBtnClick(Sender: TObject);
    procedure HidHideRunBtnClick(Sender: TObject);
  private
    procedure DefaultHandler(var Message); override;
    procedure OpenUtilityOrFolder(FilePath: string);
    function HidHideCLI(Params: string): boolean;
  protected
    procedure IconMouse(var Msg: TMessage); message WM_USER + 1;
    { Private declarations }
  public
    { Public declarations }
  end;

var
  Main: TMain;
  WM_TASKBARCREATED: Cardinal;
  DSAdvanceStarted: boolean;
  IconStarted: TIcon;
  DSAdvanceTitle: string;
  AppHiden: boolean;
  HidHidePath, HidHideCLIPath: string;
  SleepTimeOut: integer;
  DSAdvanceApp: HWND;

  IDS_RUN, IDS_STOP, IDS_SHOW, IDS_HIDE, IDS_UTILITY_OR_FOLDER_NOT_FOUND, IDS_LAST_UPDATE,
  IDS_HIDHIDE_NOT_FOUND, IDS_HIDHIDE_CLOAK_ON, IDS_DONE, IDS_HIDHIDE_ADDED, IDS_HIDHIDE_REMOVED,
  IDS_HIDHIDE_CLEARED: string;

  Utility1Path, Utility2Path, Utility3Path, Utility4Path, Utility5Path,
  Utility6Path, Utility7Path, Utility8Path, Utility9Path, Utility10Path: string;

implementation

{$R *.dfm}

type TTrayAction = (TrayAdd, TrayUpdate, TrayDelete);
procedure Tray(TrayAction: TTrayAction);
var
  NIM: TNotifyIconData;
begin
  with NIM do begin
    cbSize:=SizeOf(NIM);
    Wnd:=Main.Handle;
    uId:=1;
    uFlags:=NIF_MESSAGE or NIF_ICON or NIF_TIP;
    if DSAdvanceStarted = false then
      hIcon:=SendMessage(Main.Handle, WM_GETICON, ICON_SMALL2, 0)
    else
      hIcon:=IconStarted.Handle;
    uCallBackMessage:=WM_USER + 1;
    StrCopy(szTip, PChar(Application.Title));
  end;
  case TrayAction of
    TrayAdd: Shell_NotifyIcon(NIM_ADD, @NIM);
    TrayUpdate: Shell_NotifyIcon(NIM_MODIFY, @NIM);
    TrayDelete: Shell_NotifyIcon(NIM_DELETE, @NIM);
  end;
end;

procedure TMain.DefaultHandler(var Message);
begin
  if TMessage(Message).Msg = WM_TASKBARCREATED then
    Tray(TrayAdd);
  inherited;
end;

function GetLocaleInformation(Flag: Integer): string;
var
  pcLCA: array [0..20] of Char;
begin
  if GetLocaleInfo(LOCALE_SYSTEM_DEFAULT, Flag, pcLCA, 19)<=0 then
    pcLCA[0]:=#0;
  Result:=pcLCA;
end;

function GetHidHidePath: string;
const
  KEY_WOW64_64KEY = $0100;
var
  HKey: Windows.HKEY;
  Res: LongInt;
  Buf: array[0..1023] of AnsiChar;
  BufSize: DWORD;
  ValType: DWORD;
begin
  Result:='';
  Res:=RegOpenKeyExA(HKEY_LOCAL_MACHINE, 'SOFTWARE\Nefarius Software Solutions e.U.\HidHide', 0, KEY_READ or KEY_WOW64_64KEY, HKey);
  if Res <> ERROR_SUCCESS then Exit;
  try
    BufSize:=SizeOf(Buf);
    ValType:=0;
    Res:=RegQueryValueExA(HKey, 'Path', nil, @valType, @Buf[0], @BufSize);
    if (Res = ERROR_SUCCESS) and (ValType = REG_SZ) and (BufSize > 0) then begin
      Buf[BufSize-1]:=#0; // защита
      Result:=StrPas(Buf);
    end;
  finally
    RegCloseKey(HKey);
  end;
end;

procedure TMain.FormCreate(Sender: TObject);
var
  Ini: TIniFile;
begin
  Ini:=TIniFile.Create(ExtractFilePath(ParamStr(0)) + 'Config.ini');
  RunInBgBtn.Checked:=Ini.ReadBool('Launcher', 'RunInBackground', false);
  AutostartBtn.Checked:=Ini.ReadBool('Launcher', 'Autostart', false);
  HideGamepadBtn.Checked:=Ini.ReadBool('Launcher', 'HideGamepad', false);
  DSAdvanceTitle:=Ini.ReadString('Launcher', 'DSAdvanceTitle', 'DSAdvance');
  HidHidePath:=GetHidHidePath;
  if not DirectoryExists(HidHidePath) then
    HidHidePath:=''
  else begin
    HidHideCLIPath:=HidHidePath + '\x64\HidHideCLI.exe';
    if not FileExists(HidHideCLIPath) then
      HidHideCLIPath:='';
  end;

  SleepTimeOut:=Ini.ReadInteger('Gamepad', 'SleepTimeOut', 1) * 2;

  Utility1Path:=Ini.ReadString('Launcher', 'UtilityPath1', '');
  Utility1Btn.Caption:=Ini.ReadString('Launcher', 'UtilityTitle1', '');
  Utility2Path:=Ini.ReadString('Launcher', 'UtilityPath2', '');
  Utility2Btn.Caption:=Ini.ReadString('Launcher', 'UtilityTitle2', '');
  Utility3Path:=Ini.ReadString('Launcher', 'UtilityPath3', '');
  Utility3Btn.Caption:=Ini.ReadString('Launcher', 'UtilityTitle3', '');
  Utility4Path:=Ini.ReadString('Launcher', 'UtilityPath4', '');
  Utility4Btn.Caption:=Ini.ReadString('Launcher', 'UtilityTitle4', '');
  Utility5Path:=Ini.ReadString('Launcher', 'UtilityPath5', '');
  Utility5Btn.Caption:=Ini.ReadString('Launcher', 'UtilityTitle5', '');
  Utility6Path:=Ini.ReadString('Launcher', 'UtilityPath6', '');
  Utility6Btn.Caption:=Ini.ReadString('Launcher', 'UtilityTitle6', '');
  Utility7Path:=Ini.ReadString('Launcher', 'UtilityPath7', '');
  Utility7Btn.Caption:=Ini.ReadString('Launcher', 'UtilityTitle7', '');
  Utility8Path:=Ini.ReadString('Launcher', 'UtilityPath8', '');
  Utility8Btn.Caption:=Ini.ReadString('Launcher', 'UtilityTitle8', '');
  Utility9Path:=Ini.ReadString('Launcher', 'UtilityPath9', '');
  Utility9Btn.Caption:=Ini.ReadString('Launcher', 'UtilityTitle9', '');
  Utility10Path:=Ini.ReadString('Launcher', 'UtilityPath10', '');
  Utility10Btn.Caption:=Ini.ReadString('Launcher', 'UtilityTitle10', '');

  UtilitiesBtn.Visible:=(Utility1Path <> '') or (Utility2Path <> '') or (Utility3Path <> '') or (Utility4Path <> '')  or (Utility5Path <> '') or
                        (Utility6Path <> '') or (Utility7Path <> '') or (Utility8Path <> '') or (Utility9Path <> '')  or (Utility10Path <> '');
  N3.Visible:=UtilitiesBtn.Visible;
  Utility1Btn.Visible:=Utility1Path <> '';
  Utility2Btn.Visible:=Utility2Path <> '';
  Utility3Btn.Visible:=Utility3Path <> '';
  Utility4Btn.Visible:=Utility4Path <> '';
  Utility5Btn.Visible:=Utility5Path <> '';
  Utility6Btn.Visible:=Utility6Path <> '';
  Utility7Btn.Visible:=Utility7Path <> '';
  Utility8Btn.Visible:=Utility8Path <> '';
  Utility9Btn.Visible:=Utility9Path <> '';
  Utility10Btn.Visible:=Utility10Path <> '';
  Ini.Free;

  Application.Title:=Caption;
  WM_TASKBARCREATED:=RegisterWindowMessage('TaskbarCreated');

  IconStarted:=TIcon.Create;
  IconStarted.LoadFromFile('Launched.ico');
  DSAdvanceStarted:=false;

  Tray(TrayAdd);
  SetWindowLong(Application.Handle, GWL_EXSTYLE, GetWindowLong(Application.Handle, GWL_EXSTYLE) or WS_EX_TOOLWINDOW);

  if GetLocaleInformation(LOCALE_SENGLANGUAGE) = 'Russian' then begin
    IDS_RUN:='Запустить';
    IDS_STOP:='Остановить';
    IDS_SHOW:='Показать';
    IDS_HIDE:='Скрыть';
    IDS_UTILITY_OR_FOLDER_NOT_FOUND:='Утилита или папка не найдена. Измените путь в конфигурационном файле.';
    IDS_HIDHIDE_NOT_FOUND:='Утилита HidHide не установлена. Установите её и попробуйте снова.';
    IDS_HIDHIDE_CLOAK_ON:='Для скрытия контроллера также откройте программу HidHide, перейдите на вкладку "Devices" и отметьте галочкой свой контроллер, например "Sony Wireless Controller" или "Nintendo Pro Controller".';
    IDS_DONE:='Готово';
    IDS_HIDHIDE_ADDED:='Добавлена в исключения HidHide';
    IDS_HIDHIDE_REMOVED:='Удалено из исклчений HidHide';
    IDS_HIDHIDE_CLEARED:='Отсутствующие записи очищены';
    if Utility1Btn.Caption = '' then
      Utility1Btn.Caption:='Утилита 1';
    if Utility2Btn.Caption = '' then
      Utility2Btn.Caption:='Утилита 2';
    if Utility3Btn.Caption = '' then
      Utility3Btn.Caption:='Утилита 3';
    if Utility4Btn.Caption = '' then
      Utility4Btn.Caption:='Утилита 4';
    if Utility5Btn.Caption = '' then
      Utility5Btn.Caption:='Утилита 5';
    if Utility6Btn.Caption = '' then
      Utility6Btn.Caption:='Утилита 6';
    if Utility7Btn.Caption = '' then
      Utility7Btn.Caption:='Утилита 7';
    if Utility8Btn.Caption = '' then
      Utility8Btn.Caption:='Утилита 8';
    if Utility9Btn.Caption = '' then
      Utility9Btn.Caption:='Утилита 9';
    if Utility10Btn.Caption = '' then
      Utility10Btn.Caption:='Утилита 10';
  end else begin
    IDS_RUN:='Run';
    IDS_STOP:='Stop';
    RunStopBtn.Caption:=IDS_RUN;
    IDS_SHOW:='Show';
    IDS_HIDE:='Hide';
    IDS_UTILITY_OR_FOLDER_NOT_FOUND:='The utility or folder was not found. Change the path in the configuration file.';
    IDS_HIDHIDE_NOT_FOUND:='HidHide tool not installed. Install it and try again.';
    IDS_HIDHIDE_CLOAK_ON:='To hide the controller, also open the HidHide program, go to the "Devices" tab, and check your controller — for example, "Sony Wireless Controller" or "Nintendo Pro Controller".';
    IDS_DONE := 'Done';
    IDS_HIDHIDE_ADDED := 'Added to HidHide exceptions';
    IDS_HIDHIDE_REMOVED := 'Removed from HidHide exceptions';
    IDS_HIDHIDE_CLEARED := 'Absent entries cleared';
    SetupBtn.Caption:='Setup';
    ConfigBtn.Caption:='Options';
    KMProfilesBtn.Caption:='Keyboard && Mouse Profiles';
    XboxProfilesBtn.Caption:='Xbox Gamepad Profiles';
    RunInBgBtn.Caption:='Run in background';
    HidHideSubMenu.Caption:='Hide Gamepad (HidHide)';
    HideGamepadBtn.Caption:='Hide from Games & Apps';
    HidHideRunBtn.Caption:='Run HidHide';
    HidHideAddBtn.Caption:='Add to Exceptions';
    HidHideRemBtn.Caption:='Remove from Exceptions';
    HideHideClearBtn.Caption := 'Clear Absent';
    GamepadTestBtn.Caption:='Gamepad test';
    AutostartBtn.Caption:='Autostart';
    CloseBtn.Caption:='Exit';
    UtilitiesBtn.Caption:='Utilities';
    if Utility1Btn.Caption = '' then
      Utility1Btn.Caption:='Utility 1';
    if Utility2Btn.Caption = '' then
      Utility2Btn.Caption:='Utility 2';
    if Utility3Btn.Caption = '' then
      Utility3Btn.Caption:='Utility 3';
    if Utility4Btn.Caption = '' then
      Utility4Btn.Caption:='Utility 4';
    if Utility5Btn.Caption = '' then
      Utility5Btn.Caption:='Utility 5';
    if Utility6Btn.Caption = '' then
      Utility6Btn.Caption:='Utility 6';
    if Utility7Btn.Caption = '' then
      Utility7Btn.Caption:='Utility 7';
    if Utility8Btn.Caption = '' then
      Utility8Btn.Caption:='Utility 8';
    if Utility9Btn.Caption = '' then
      Utility9Btn.Caption:='Utility 9';
    if Utility10Btn.Caption = '' then
      Utility10Btn.Caption:='Utility 10';
    OpenDialog.Filter:='Apps|*.exe';
  end;

  if AutostartBtn.Checked then
    RunStopBtn.Click;
end;

procedure TMain.IconMouse(var Msg: TMessage);
begin
  case Msg.LParam of
    WM_LBUTTONDBLCLK: RunStopBtn.Click;
    WM_LBUTTONDOWN: begin
      PostMessage(Handle, WM_LBUTTONDOWN, MK_LBUTTON, 0);
      PostMessage(Handle, WM_LBUTTONUP, MK_LBUTTON, 0);
    end;
    WM_RBUTTONDOWN:
    begin
      SetForegroundWindow(Handle);
      TrayPopupMenu.Popup(Mouse.CursorPos.X, Mouse.CursorPos.Y);
    end;
  end;
end;

procedure TMain.CloseBtnClick(Sender: TObject);
begin
  Close;
end;

procedure TMain.FormClose(Sender: TObject; var Action: TCloseAction);
begin
  Tray(TrayDelete);
  IconStarted.Free;
end;

procedure TMain.RunInBgBtnClick(Sender: TObject);
var
  Ini: TIniFile;
begin
  RunInBgBtn.Checked:=not RunInBgBtn.Checked;
  Ini:=TIniFile.Create(ExtractFilePath(ParamStr(0)) + 'Config.ini');
  Ini.WriteBool('Launcher', 'RunInBackground', RunInBgBtn.Checked);
  Ini.Free;
end;

procedure TMain.RunStopBtnClick(Sender: TObject);
begin
  if DSAdvanceStarted = false then begin
    CheckAppClosedTimer.Enabled:=true;
    DSAdvanceStarted:=true;
    RunStopBtn.Caption:=IDS_STOP;
    if RunInBgBtn.Checked then begin
      ShellExecute(Handle, 'open', PChar(ExtractFilePath(ParamStr(0)) + 'DSAdvance.exe'), nil, nil, SW_HIDE);
      ShowHideAppBtn.Visible:=true;
      ShowHideAppBtn.Caption:=IDS_SHOW;
      AppHiden:=true;
    end else
      ShellExecute(Handle, 'open', PChar(ExtractFilePath(ParamStr(0)) + 'DSAdvance.exe'), nil, nil, SW_SHOWNORMAL);
  end else begin
    DSAdvanceStarted:=false;
    AppHiden:=false;
    RunStopBtn.Caption:=IDS_RUN;

    DSAdvanceApp:=FindWindow(nil, PChar(DSAdvanceTitle));
    if (DSAdvanceApp <> 0) then begin // Не эмулируем нажатие кнопок, поскольку эта комбинация может переключить окно, если приложение закрыто
      keybd_event(VK_MENU, 0, 0, 0);
      keybd_event(VK_ESCAPE, 0, 0, 0);
      Sleep(SleepTimeOut);
      keybd_event(VK_ESCAPE, 0, KEYEVENTF_KEYUP, 0);
      keybd_event(VK_MENU, 0, KEYEVENTF_KEYUP, 0);
      Sleep(1);
      WinExec('taskkill /f /im DSAdvance.exe', SW_HIDE); // На всякий случай
    end;

    if RunInBgBtn.Checked then ShowHideAppBtn.Visible:=false;
  end;
  Tray(TrayUpdate);
end;

procedure TMain.ConfigBtnClick(Sender: TObject);
begin
  ShellExecute(Handle, 'open', PChar(ExtractFilePath(ParamStr(0)) + 'Config.ini'), nil, nil, SW_SHOWNORMAL);
end;

procedure TMain.GamepadTestBtnClick(Sender: TObject);
begin
  ShellExecute(Handle, 'open', PChar(ExtractFilePath(ParamStr(0)) + 'XInputTest.exe'), nil, nil, SW_SHOWNORMAL);
end;

procedure TMain.ShowHideAppBtnClick(Sender: TObject);
begin
  DSAdvanceApp:=FindWindow(nil, PChar(DSAdvanceTitle));
  if DSAdvanceApp = 0 then begin
    ShowHideAppBtn.Visible:=false;
    Exit;
  end;

  if AppHiden then begin
    ShowWindow(DSAdvanceApp, SW_SHOW);
    ShowHideAppBtn.Caption:=IDS_HIDE;
    AppHiden:=false;
  end else begin
    ShowWindow(DSAdvanceApp, SW_HIDE);
    ShowHideAppBtn.Caption:=IDS_SHOW;
    AppHiden:=true;
  end;
end;

procedure TMain.CheckAppClosedTimerTimer(Sender: TObject);
begin
  DSAdvanceApp:=FindWindow(nil, PChar(DSAdvanceTitle));
  if (DSAdvanceApp = 0) and (DSAdvanceStarted) then begin
    RunStopBtn.Click;
    CheckAppClosedTimer.Enabled:=false;
  end;
end;

procedure TMain.KMProfilesBtnClick(Sender: TObject);
begin
  ShellExecute(Handle, 'open', PChar(ExtractFilePath(ParamStr(0)) + 'KMProfiles\'), nil, nil, SW_SHOWNORMAL);
end;

procedure TMain.OpenUtilityOrFolder(FilePath: string);
begin
  if (FilePath = '') or (not (FileExists(FilePath) or DirectoryExists(FilePath))) then
    Application.MessageBox(PChar(IDS_UTILITY_OR_FOLDER_NOT_FOUND), PChar(Caption), MB_ICONWARNING)
  else
    ShellExecute(Handle, 'open', PChar(FilePath), nil, nil, SW_SHOWNORMAL);
end;

procedure TMain.Utility1BtnClick(Sender: TObject);
begin
  OpenUtilityOrFolder(Utility1Path);
end;

procedure TMain.Utility2BtnClick(Sender: TObject);
begin
  OpenUtilityOrFolder(Utility2Path);
end;

procedure TMain.Utility3BtnClick(Sender: TObject);
begin
  OpenUtilityOrFolder(Utility3Path);
end;

procedure TMain.Utility4BtnClick(Sender: TObject);
begin
  OpenUtilityOrFolder(Utility4Path);
end;

procedure TMain.Utility5BtnClick(Sender: TObject);
begin
  OpenUtilityOrFolder(Utility5Path);
end;

procedure TMain.Utility6BtnClick(Sender: TObject);
begin
  OpenUtilityOrFolder(Utility6Path);
end;

procedure TMain.Utility7BtnClick(Sender: TObject);
begin
  OpenUtilityOrFolder(Utility7Path);
end;

procedure TMain.Utility8BtnClick(Sender: TObject);
begin
  OpenUtilityOrFolder(Utility8Path);
end;

procedure TMain.Utility9BtnClick(Sender: TObject);
begin
  OpenUtilityOrFolder(Utility9Path);
end;

procedure TMain.Utility10BtnClick(Sender: TObject);
begin
  OpenUtilityOrFolder(Utility10Path);
end;

procedure TMain.AutostartBtnClick(Sender: TObject);
var
  Ini: TIniFile;
begin
  AutostartBtn.Checked:=not AutostartBtn.Checked;
  Ini:=TIniFile.Create(ExtractFilePath(ParamStr(0)) + 'Config.ini');
  Ini.WriteBool('Launcher', 'Autostart', AutostartBtn.Checked);
  Ini.Free;
end;

procedure TMain.XboxProfilesBtnClick(Sender: TObject);
begin
  ShellExecute(Handle, 'open', PChar(ExtractFilePath(ParamStr(0)) + 'XboxProfiles\'), nil, nil, SW_SHOWNORMAL);
end;

procedure TMain.HidHideAddBtnClick(Sender: TObject);
var
  ProcessStatus: Cardinal;
begin
  if not OpenDialog.Execute then Exit;

  if HidHideCLI('--app-reg "' + OpenDialog.FileName + '"') then
    MessageBox(0, PChar(IDS_HIDHIDE_ADDED), PChar(Caption), MB_ICONINFORMATION);
end;

procedure TMain.HideHideClearBtnClick(Sender: TObject);
var
  ProcessStatus: Cardinal;
begin
  if HidHideCLI('--app-clean') then
    MessageBox(0, PChar(IDS_HIDHIDE_CLEARED), PChar(Caption), MB_ICONINFORMATION);
end;

procedure TMain.HideGamepadBtnClick(Sender: TObject);
var
  Ini: TIniFile; HidHideDone: boolean;
begin
  HideGamepadBtn.Checked:=not HideGamepadBtn.Checked;

  Ini:=TIniFile.Create(ExtractFilePath(ParamStr(0)) + 'Config.ini');
  Ini.WriteBool('Launcher', 'HideGamepad', HideGamepadBtn.Checked);
  Ini.Free;

  if HideGamepadBtn.Checked then begin
    if HidHideCLI('--cloak-on') then begin
      HidHideCLI('--app-reg "' + ExtractFilePath(ParamStr(0)) + 'DSAdvance.exe"');
      MessageBox(0, PChar(IDS_HIDHIDE_CLOAK_ON), PChar(Caption), MB_ICONINFORMATION);
    end;
  end else
    if HidHideCLI('--cloak-off') then
      MessageBox(0, PChar(IDS_DONE), PChar(Caption), MB_ICONINFORMATION);
end;

procedure TMain.HidHideRemBtnClick(Sender: TObject);
var
  ProcessStatus: Cardinal;
begin
  if not OpenDialog.Execute then Exit;

  if HidHideCLI('--app-unreg "' + OpenDialog.FileName + '"') then
      MessageBox(0, PChar(IDS_HIDHIDE_REMOVED), PChar(Caption), MB_ICONINFORMATION);
end;

function TMain.HidHideCLI(Params: string): boolean;
begin
  if HidHideCLIPath <> '' then begin
    ShellExecute(Handle, 'runas', PChar(HidHideCLIPath), PChar(Params), nil, SW_SHOWNORMAL);
    Result:=true;
  end else begin
    Application.MessageBox(PChar(IDS_HIDHIDE_NOT_FOUND), PChar(Caption), MB_ICONWARNING);
    Result:=false;
  end;
end;

procedure TMain.HidHideRunBtnClick(Sender: TObject);
begin
  OpenUtilityOrFolder(HidHidePath + '\x64\HidHideClient.exe');
end;

end.
