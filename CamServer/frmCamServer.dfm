object CamServerFrm: TCamServerFrm
  Left = 0
  Top = 0
  BorderStyle = bsToolWindow
  Caption = 'ESP Cam Server'
  ClientHeight = 87
  ClientWidth = 450
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -12
  Font.Name = 'Segoe UI'
  Font.Style = []
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
    object txtPort: TEdit
      Left = 144
      Top = 8
      Width = 80
      Height = 23
      TabOrder = 1
      Text = '5001'
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
    ExplicitLeft = 216
    ExplicitTop = 64
    ExplicitWidth = 0
  end
  object sckServer: TWSocket
    LineEnd = #13#10
    Port = '5001'
    Proto = 'tcp'
    LocalAddr = '0.0.0.0'
    LocalAddr6 = '::'
    SocksLevel = '5'
    ExclusiveAddr = False
    ComponentOptions = []
    ListenBacklog = 15
    OnSessionAvailable = sckServerSessionAvailable
    SocketErrs = wsErrTech
    Left = 376
    Top = 16
  end
end
