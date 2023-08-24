object MainFrm: TMainFrm
  Left = 0
  Top = 0
  BorderIcons = [biMinimize, biMaximize]
  BorderStyle = bsSingle
  Caption = 'UDP Packet Sender...'
  ClientHeight = 441
  ClientWidth = 624
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -12
  Font.Name = 'Segoe UI'
  Font.Style = []
  Position = poDesktopCenter
  OnClose = FormClose
  OnCreate = FormCreate
  TextHeight = 15
  object Panel1: TPanel
    Left = 0
    Top = 0
    Width = 624
    Height = 25
    Align = alTop
    TabOrder = 0
  end
  object Panel2: TPanel
    Left = 0
    Top = 400
    Width = 624
    Height = 41
    Align = alBottom
    TabOrder = 1
    object lblPort: TLabel
      Left = 24
      Top = 16
      Width = 22
      Height = 15
      Caption = 'Port'
    end
    object edPort: TEdit
      Left = 52
      Top = 8
      Width = 81
      Height = 23
      TabOrder = 0
      Text = '3000'
    end
    object btnStart: TButton
      Left = 152
      Top = 6
      Width = 75
      Height = 25
      Caption = 'Start'
      TabOrder = 1
      OnClick = btnStartClick
    end
    object btnClose: TButton
      Left = 536
      Top = 8
      Width = 75
      Height = 25
      Caption = 'Close'
      TabOrder = 2
      OnClick = btnCloseClick
    end
  end
  object memDisplay: TMemo
    Left = 0
    Top = 25
    Width = 624
    Height = 375
    Align = alClient
    TabOrder = 2
    ExplicitTop = 27
  end
  object sockUdp: TWSocket
    LineEnd = #13#10
    Addr = '0.0.0.0'
    Port = '3000'
    Proto = 'udp'
    LocalAddr = '0.0.0.0'
    LocalAddr6 = '::'
    LocalPort = '0'
    SocksLevel = '5'
    ExclusiveAddr = False
    ComponentOptions = []
    ListenBacklog = 15
    OnDataAvailable = sockUdpDataAvailable
    OnChangeState = sockUdpChangeState
    SocketErrs = wsErrTech
    Left = 464
    Top = 96
  end
  object tmrSend: TTimer
    Enabled = False
    Interval = 5000
    OnTimer = tmrSendTimer
    Left = 496
    Top = 320
  end
end
