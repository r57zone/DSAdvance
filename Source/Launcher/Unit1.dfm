object Main: TMain
  Left = 192
  Top = 125
  Width = 336
  Height = 279
  Caption = 'DSAdvance'
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'MS Sans Serif'
  Font.Style = []
  OldCreateOrder = False
  OnClose = FormClose
  OnCreate = FormCreate
  PixelsPerInch = 96
  TextHeight = 13
  object TrayPopupMenu: TPopupMenu
    Left = 8
    Top = 8
    object RunStopBtn: TMenuItem
      Caption = #1047#1072#1087#1091#1089#1090#1080#1090#1100
      OnClick = RunStopBtnClick
    end
    object ShowHideAppBtn: TMenuItem
      Caption = #1055#1086#1082#1072#1079#1072#1090#1100
      Visible = False
      OnClick = ShowHideAppBtnClick
    end
    object N1: TMenuItem
      Caption = '-'
    end
    object SetupBtn: TMenuItem
      Caption = #1053#1072#1089#1090#1088#1086#1081#1082#1072
      object ConfigBtn: TMenuItem
        Caption = #1055#1072#1088#1072#1084#1077#1090#1088#1099
        OnClick = ConfigBtnClick
      end
      object N4: TMenuItem
        Caption = '-'
      end
      object XboxProfilesBtn: TMenuItem
        Caption = #1055#1088#1086#1092#1080#1083#1080' Xbox '#1075#1077#1081#1084#1087#1072#1076#1072
        OnClick = XboxProfilesBtnClick
      end
      object KMProfilesBtn: TMenuItem
        Caption = #1055#1088#1086#1092#1080#1083#1080' '#1082#1083#1072#1074'. '#1080' '#1084#1099#1096#1080
        OnClick = KMProfilesBtnClick
      end
      object N5: TMenuItem
        Caption = '-'
      end
      object HidHideSubMenu: TMenuItem
        Caption = #1057#1082#1088#1099#1090#1080#1077' '#1075#1077#1081#1084#1087#1072#1076#1072' (HidHide)'
        object HideGamepadBtn: TMenuItem
          Caption = #1057#1082#1088#1099#1090#1100' '#1086#1090' '#1080#1075#1088' '#1080' '#1087#1088#1080#1083#1086#1078#1077#1085#1080#1081
          OnClick = HideGamepadBtnClick
        end
        object N9: TMenuItem
          Caption = '-'
        end
        object HidHideRunBtn: TMenuItem
          Caption = #1047#1072#1087#1091#1089#1090#1080#1090#1100' HidHide'
          OnClick = HidHideRunBtnClick
        end
        object N10: TMenuItem
          Caption = '-'
        end
        object HidHideAddBtn: TMenuItem
          Caption = #1044#1086#1073#1072#1074#1080#1090#1100' '#1074' '#1080#1089#1082#1083#1102#1095#1077#1085#1080#1103
          OnClick = HidHideAddBtnClick
        end
        object HidHideRemBtn: TMenuItem
          Caption = #1059#1076#1072#1083#1080#1090#1100' '#1080#1079' '#1080#1089#1082#1083#1102#1095#1077#1085#1080#1081
          OnClick = HidHideRemBtnClick
        end
        object N7: TMenuItem
          Caption = '-'
        end
        object HideHideClearBtn: TMenuItem
          Caption = #1054#1095#1080#1089#1090#1080#1090#1100' '#1086#1090#1089#1091#1090#1089#1090#1074#1091#1102#1097#1077#1077
          OnClick = HideHideClearBtnClick
        end
      end
      object GamepadTestBtn: TMenuItem
        Caption = #1055#1088#1086#1074#1077#1088#1082#1072' '#1075#1077#1081#1084#1087#1072#1076#1072
        OnClick = GamepadTestBtnClick
      end
      object N6: TMenuItem
        Caption = '-'
      end
      object RunInBgBtn: TMenuItem
        Caption = #1047#1072#1087#1091#1089#1082#1072#1090#1100' '#1074' '#1092#1086#1085#1077
        OnClick = RunInBgBtnClick
      end
      object AutostartBtn: TMenuItem
        Caption = #1040#1074#1090#1086#1079#1072#1087#1091#1089#1082
        OnClick = AutostartBtnClick
      end
    end
    object N2: TMenuItem
      Caption = '-'
    end
    object UtilitiesBtn: TMenuItem
      Caption = #1059#1090#1080#1083#1080#1090#1099
      Visible = False
      object Utility1Btn: TMenuItem
        Caption = #1059#1090#1080#1083#1080#1090#1072' 1'
        Visible = False
        OnClick = Utility1BtnClick
      end
      object Utility2Btn: TMenuItem
        Caption = #1059#1090#1080#1083#1080#1090#1072' 2'
        Visible = False
        OnClick = Utility2BtnClick
      end
      object Utility3Btn: TMenuItem
        Caption = #1059#1090#1080#1083#1080#1090#1072' 3'
        Visible = False
        OnClick = Utility3BtnClick
      end
      object Utility4Btn: TMenuItem
        Caption = #1059#1090#1080#1083#1080#1090#1072' 4'
        Visible = False
        OnClick = Utility4BtnClick
      end
      object Utility5Btn: TMenuItem
        Caption = #1059#1090#1080#1083#1080#1090#1072' 5'
        OnClick = Utility5BtnClick
      end
      object Utility6Btn: TMenuItem
        Caption = #1059#1090#1080#1083#1080#1090#1072' 6'
        OnClick = Utility6BtnClick
      end
      object Utility7Btn: TMenuItem
        Caption = #1059#1090#1080#1083#1080#1090#1072' 7'
        OnClick = Utility7BtnClick
      end
      object Utility8Btn: TMenuItem
        Caption = #1059#1090#1080#1083#1080#1090#1072' 8'
        OnClick = Utility8BtnClick
      end
      object Utility9Btn: TMenuItem
        Caption = #1059#1090#1080#1083#1080#1090#1072' 9'
        OnClick = Utility9BtnClick
      end
      object Utility10Btn: TMenuItem
        Caption = #1059#1090#1080#1083#1080#1090#1072' 10'
        OnClick = Utility10BtnClick
      end
    end
    object N3: TMenuItem
      Caption = '-'
      Visible = False
    end
    object CloseBtn: TMenuItem
      Caption = #1042#1099#1093#1086#1076
      OnClick = CloseBtnClick
    end
  end
  object CheckAppClosedTimer: TTimer
    Enabled = False
    Interval = 60000
    OnTimer = CheckAppClosedTimerTimer
    Left = 72
    Top = 8
  end
  object XPManifest1: TXPManifest
    Left = 40
    Top = 8
  end
  object OpenDialog: TOpenDialog
    Filter = #1055#1088#1080#1083#1086#1078#1077#1085#1080#1103'|*.exe'
    Left = 104
    Top = 8
  end
end
