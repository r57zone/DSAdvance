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
  object PopupMenu1: TPopupMenu
    Left = 8
    Top = 8
    object RunStopBtn: TMenuItem
      Caption = #1047#1072#1087#1091#1089#1090#1080#1090#1100
      OnClick = RunStopBtnClick
    end
    object N3: TMenuItem
      Caption = '-'
    end
    object SetupBtn: TMenuItem
      Caption = #1053#1072#1089#1090#1088#1086#1081#1082#1072
      object ConfigBtn: TMenuItem
        Caption = #1055#1072#1088#1072#1084#1077#1090#1088#1099
        OnClick = ConfigBtnClick
      end
      object N1: TMenuItem
        Caption = '-'
      end
      object RunInBgBtn: TMenuItem
        Caption = #1047#1072#1087#1091#1089#1082#1072#1090#1100' '#1074' '#1092#1086#1085#1077
        OnClick = RunInBgBtnClick
      end
    end
    object N2: TMenuItem
      Caption = '-'
    end
    object CloseBtn: TMenuItem
      Caption = #1042#1099#1093#1086#1076
      OnClick = CloseBtnClick
    end
  end
  object XPManifest1: TXPManifest
    Left = 40
    Top = 8
  end
end