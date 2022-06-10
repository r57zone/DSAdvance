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
    N3: TMenuItem;
    RunStopBtn: TMenuItem;
    RunInBgBtn: TMenuItem;
    SetupBtn: TMenuItem;
    N1: TMenuItem;
    ConfigBtn: TMenuItem;
    N4: TMenuItem;
    GamepadTestBtn: TMenuItem;
    ShowHideAppBtn: TMenuItem;
    CheckAppClosedTimer: TTimer;
    XPManifest1: TXPManifest;
    procedure FormCreate(Sender: TObject);
    procedure CloseBtnClick(Sender: TObject);
    procedure FormClose(Sender: TObject; var Action: TCloseAction);
    procedure RunInBgBtnClick(Sender: TObject);
    procedure RunStopBtnClick(Sender: TObject);
    procedure ConfigBtnClick(Sender: TObject);
    procedure GamepadTestBtnClick(Sender: TObject);
    procedure ShowHideAppBtnClick(Sender: TObject);
    procedure CheckAppClosedTimerTimer(Sender: TObject);
  private
    procedure DefaultHandler(var Message); override;
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

  IDS_RUN, IDS_STOP, IDS_SHOW, IDS_HIDE, IDS_LAST_UPDATE: string;

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
  end else begin
    IDS_RUN:='Run';
    IDS_STOP:='Stop';
    RunStopBtn.Caption:=IDS_RUN;
    IDS_SHOW:='Show';
    IDS_HIDE:='Hide';
    SetupBtn.Caption:='Setup';
    ConfigBtn.Caption:='Options';
    RunInBgBtn.Caption:='Run in background';
    GamepadTestBtn.Caption:='Gamepad test';
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
    WinExec('taskkill /f /im DSAdvance.exe', SW_HIDE);
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
var
  DSAdvanceApp: HWND;
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
var
  DSAdvanceApp: HWND;
begin
  DSAdvanceApp:=FindWindow(nil, PChar(DSAdvanceTitle));
  if (DSAdvanceApp = 0) and (DSAdvanceStarted) then begin
    RunStopBtn.Click;
    CheckAppClosedTimer.Enabled:=false;
  end;
end;

end.
