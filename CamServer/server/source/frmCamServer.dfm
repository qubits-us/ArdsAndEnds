object CamServerFrm: TCamServerFrm
  Left = 0
  Top = 0
  BorderIcons = [biSystemMenu, biMinimize]
  BorderStyle = bsSingle
  Caption = 'ESP Cam Server'
  ClientHeight = 87
  ClientWidth = 450
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -12
  Font.Name = 'Segoe UI'
  Font.Style = []
  Menu = MainMenu1
  OnClose = FormClose
  OnCreate = FormCreate
  TextHeight = 15
  object pnlTop: TPanel
    Left = 0
    Top = 0
    Width = 450
    Height = 38
    Align = alTop
    TabOrder = 0
    object lblPort: TLabel
      Left = 116
      Top = 10
      Width = 22
      Height = 15
      Caption = 'Port'
    end
    object btnCtrl: TButton
      Left = 8
      Top = 6
      Width = 75
      Height = 25
      Caption = 'Listen'
      TabOrder = 0
      OnClick = btnCtrlClick
    end
    object edPort: TEdit
      Left = 144
      Top = 7
      Width = 80
      Height = 23
      TabOrder = 1
      Text = '50001'
    end
    object btnPlayback: TButton
      Left = 364
      Top = 8
      Width = 75
      Height = 25
      Caption = 'Playback'
      TabOrder = 2
      OnClick = btnPlaybackClick
    end
  end
  object sbMain: TStatusBar
    Left = 0
    Top = 68
    Width = 450
    Height = 19
    Panels = <
      item
        Text = 'Connections: 0'
        Width = 50
      end>
  end
  object sckServer: TWSocket
    LineEnd = #13#10
    Port = '5001'
    Proto = 'tcp'
    LocalAddr = '0.0.0.0'
    LocalAddr6 = '::'
    LocalPort = '0'
    SocksLevel = '5'
    ExclusiveAddr = False
    ComponentOptions = [wsoTcpNoDelay]
    ListenBacklog = 15
    OnSessionAvailable = sckServerSessionAvailable
    SocketErrs = wsErrTech
    Left = 408
    Top = 40
  end
  object MainMenu1: TMainMenu
    Left = 360
    Top = 40
    object File1: TMenuItem
      Caption = '&File'
      object Exit1: TMenuItem
        Caption = '&Exit'
        OnClick = Exit1Click
      end
    end
    object Cams1: TMenuItem
      Caption = '&Cams'
      Visible = False
      object List1: TMenuItem
        Caption = '&List'
      end
      object Firmwares1: TMenuItem
        Caption = 'F&irmwares'
      end
    end
    object Help1: TMenuItem
      Caption = '&Help'
      object About1: TMenuItem
        Caption = '&About'
        OnClick = About1Click
      end
    end
  end
  object sckDiscv: TWSocket
    LineEnd = #13#10
    Port = '50000'
    Proto = 'udp'
    LocalAddr = '0.0.0.0'
    LocalAddr6 = '::'
    LocalPort = '0'
    SocksLevel = '5'
    ExclusiveAddr = False
    ComponentOptions = []
    ListenBacklog = 15
    SocketErrs = wsErrTech
    Left = 312
    Top = 40
  end
  object tmrDiscv: TTimer
    Enabled = False
    Interval = 10000
    OnTimer = tmrDiscvTimer
    Left = 232
    Top = 40
  end
end
