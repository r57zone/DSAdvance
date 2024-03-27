unit Unit1;

interface

uses
  Windows, Messages, SysUtils, Variants, Classes, Graphics, Controls, Forms,
  Dialogs, Menus, ExtCtrls, ShellAPI, IniFiles, XPMan;

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
    ProfilesBtn: TMenuItem;
    HidHideBtn: TMenuItem;
    N3: TMenuItem;
    UtilitiesBtn: TMenuItem;
    Utility1Btn: TMenuItem;
    Utility2Btn: TMenuItem;
    Utility3Btn: TMenuItem;
    Utility4Btn: TMenuItem;
    procedure FormCreate(Sender: TObject);
    procedure CloseBtnClick(Sender: TObject);
    procedure FormClose(Sender: TObject; var Action: TCloseAction);
    procedure RunInBgBtnClick(Sender: TObject);
    procedure RunStopBtnClick(Sender: TObject);
    procedure ConfigBtnClick(Sender: TObject);
    procedure GamepadTestBtnClick(Sender: TObject);
    procedure ShowHideAppBtnClick(Sender: TObject);
    procedure CheckAppClosedTimerTimer(Sender: TObject);
    procedure ProfilesBtnClick(Sender: TObject);
    procedure HidHideBtnClick(Sender: TObject);
    procedure Utility1BtnClick(Sender: TObject);
    procedure Utility2BtnClick(Sender: TObject);
    procedure Utility3BtnClick(Sender: TObject);
    procedure Utility4BtnClick(Sender: TObject);
  private
    procedure DefaultHandler(var Message); override;
    procedure OpenUtility(FileName: string);
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
  HidHidePath: string;
  SleepTimeOut: integer;
  DSAdvanceApp: HWND;

  IDS_RUN, IDS_STOP, IDS_SHOW, IDS_HIDE, IDS_UTILITY_NOT_FOUND, IDS_LAST_UPDATE: string;

  Utility1Path, Utility2Path, Utility3Path, Utility4Path: string;

implementation

{$R *.dfm}

procedure Tray(ActInd: integer);  // 1 - Add, 2 - Update, 3 - Remove
var
  NIM: TNotifyIconData;
begin
  with NIM do begin
    cbSize:=SizeOf(NIM);
    Wnd:=Main.Handle;
    uId:=1;
    uFlags:=NIF_MESSAGE or NIF_ICON or NIF_TIP;
    
    if DSAdvanceStarted = false then
      hIcon:=SendMessage(Application.Handle, WM_GETICON, ICON_SMALL2, 0)
    else
      hIcon:=IconStarted.Handle;

    uCallBackMessage:=WM_USER + 1;
    StrCopy(szTip, PChar(Application.Title));
  end;
  case ActInd of
    1: Shell_NotifyIcon(NIM_ADD, @NIM);
    2: Shell_NotifyIcon(NIM_MODIFY, @NIM);
    3: Shell_NotifyIcon(NIM_DELETE, @NIM);
  end;
end;

procedure TMain.DefaultHandler(var Message);
begin
  if TMessage(Message).Msg = WM_TASKBARCREATED then
    Tray(1);
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

procedure TMain.FormCreate(Sender: TObject);
var
  Ini: TIniFile;
begin
  Ini:=TIniFile.Create(ExtractFilePath(ParamStr(0)) + 'Config.ini');
  if Ini.ReadBool('Launcher', 'RunInBackground', false) then RunInBgBtn.Click;
  DSAdvanceTitle:=Ini.ReadString('Launcher', 'DSAdvanceTitle', 'DSAdvance');
  HidHidePath:=Ini.ReadString('Launcher', 'HidHidePath', '');
  if not FileExists(HidHidePath) then
    HidHidePath:='';
  SleepTimeOut:=Ini.ReadInteger('Gamepad', 'SleepTimeOut', 1) * 2;

  Utility1Path:=Ini.ReadString('Launcher', 'UtilityPath1', '');
  Utility1Btn.Caption:=Ini.ReadString('Launcher', 'UtilityTitle1', '');
  Utility2Path:=Ini.ReadString('Launcher', 'UtilityPath2', '');
  Utility2Btn.Caption:=Ini.ReadString('Launcher', 'UtilityTitle2', '');
  Utility3Path:=Ini.ReadString('Launcher', 'UtilityPath3', '');
  Utility3Btn.Caption:=Ini.ReadString('Launcher', 'UtilityTitle3', '');
  Utility4Path:=Ini.ReadString('Launcher', 'UtilityPath4', '');
  Utility4Btn.Caption:=Ini.ReadString('Launcher', 'UtilityTitle4', '');

  UtilitiesBtn.Visible:=(Utility1Path <> '') or (Utility2Path <> '') or (Utility3Path <> '') or (Utility4Path <> '');
  N3.Visible:=UtilitiesBtn.Visible;
  Utility1Btn.Visible:=Utility1Path <> '';
  Utility2Btn.Visible:=Utility2Path <> '';
  Utility3Btn.Visible:=Utility3Path <> '';
  Utility4Btn.Visible:=Utility4Path <> '';
  Ini.Free;

  Application.Title:=Caption;
  WM_TASKBARCREATED:=RegisterWindowMessage('TaskbarCreated');
  
  IconStarted:=TIcon.Create;
  IconStarted.LoadFromFile('Launched.ico');
  DSAdvanceStarted:=false;

  Tray(1);
  SetWindowLong(Application.Handle, GWL_EXSTYLE, GetWindowLong(Application.Handle, GWL_EXSTYLE) or WS_EX_TOOLWINDOW);

  if GetLocaleInformation(LOCALE_SENGLANGUAGE) = 'Russian' then begin
    IDS_RUN:='Запустить';
    IDS_STOP:='Остановить';
    IDS_SHOW:='Показать';
    IDS_HIDE:='Скрыть';
    IDS_UTILITY_NOT_FOUND:='Утилита не найдена. Измените путь в конфигурационном файле.';
    if Utility1Btn.Caption = '' then
      Utility1Btn.Caption:='Утилита 1';
    if Utility2Btn.Caption = '' then
      Utility2Btn.Caption:='Утилита 2';
    if Utility3Btn.Caption = '' then
      Utility3Btn.Caption:='Утилита 3';
    if Utility4Btn.Caption = '' then
      Utility4Btn.Caption:='Утилита 4';
  end else begin
    IDS_RUN:='Run';
    IDS_STOP:='Stop';
    RunStopBtn.Caption:=IDS_RUN;
    IDS_SHOW:='Show';
    IDS_HIDE:='Hide';
    IDS_UTILITY_NOT_FOUND:='The utility was not found. Change the path in the configuration file.';
    SetupBtn.Caption:='Setup';
    ConfigBtn.Caption:='Options';
    ProfilesBtn.Caption:='Profiles';
    RunInBgBtn.Caption:='Run in background';
    GamepadTestBtn.Caption:='Gamepad test';
    UtilitiesBtn.Caption:='Utilities';
    if Utility1Btn.Caption = '' then
      Utility1Btn.Caption:='Utility 1';
    if Utility2Btn.Caption = '' then
      Utility2Btn.Caption:='Utility 2';
    if Utility3Btn.Caption = '' then
      Utility3Btn.Caption:='Utility 3';
    if Utility4Btn.Caption = '' then
      Utility4Btn.Caption:='Utility 4';
    CloseBtn.Caption:='Exit';
  end;
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
  Tray(3);
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
  Tray(2);
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

procedure TMain.ProfilesBtnClick(Sender: TObject);
begin
  ShellExecute(Handle, 'open', PChar(ExtractFilePath(ParamStr(0)) + 'Profiles\'), nil, nil, SW_SHOWNORMAL);
end;

procedure TMain.HidHideBtnClick(Sender: TObject);
begin
  OpenUtility(HidHidePath);
end;

procedure TMain.OpenUtility(FileName: string);
begin
  if (FileName = '') or (FileExists(FileName) = false) then begin
    Application.MessageBox(PChar(IDS_UTILITY_NOT_FOUND), PChar(Caption), MB_ICONWARNING);
    Exit;
  end;
  ShellExecute(Handle, 'open', PChar(FileName), nil, nil, SW_SHOWNORMAL);
end;

procedure TMain.Utility1BtnClick(Sender: TObject);
begin
  OpenUtility(Utility1Path);
end;

procedure TMain.Utility2BtnClick(Sender: TObject);
begin
  OpenUtility(Utility2Path);
end;

procedure TMain.Utility3BtnClick(Sender: TObject);
begin
  OpenUtility(Utility3Path);
end;

procedure TMain.Utility4BtnClick(Sender: TObject);
begin
  OpenUtility(Utility4Path);
end;

end.
