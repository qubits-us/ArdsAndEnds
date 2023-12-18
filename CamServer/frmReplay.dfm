object ReplayFrm: TReplayFrm
  Left = 0
  Top = 0
  Caption = 'Cam Video Replay...'
  ClientHeight = 439
  ClientWidth = 476
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -12
  Font.Name = 'Segoe UI'
  Font.Style = []
  Position = poScreenCenter
  OnClose = FormClose
  OnCreate = FormCreate
  DesignSize = (
    476
    439)
  TextHeight = 15
  object lblSelectFile: TLabel
    Left = 8
    Top = 339
    Width = 72
    Height = 15
    Anchors = [akLeft, akBottom]
    Caption = 'Selected file...'
    ExplicitTop = 336
  end
  object lblFrameInfo: TLabel
    Left = 390
    Top = 338
    Width = 29
    Height = 15
    Anchors = [akRight, akBottom]
    Caption = '0 of 0'
    ExplicitLeft = 416
    ExplicitTop = 336
  end
  object pnlTop: TPanel
    Left = 0
    Top = 0
    Width = 476
    Height = 276
    Align = alTop
    Anchors = [akLeft, akTop, akRight, akBottom]
    TabOrder = 0
    ExplicitWidth = 495
    ExplicitHeight = 273
    object im: TImage
      Left = 1
      Top = 1
      Width = 474
      Height = 274
      Align = alClient
      Stretch = True
      ExplicitWidth = 493
      ExplicitHeight = 312
    end
  end
  object edFileName: TEdit
    Left = 8
    Top = 360
    Width = 374
    Height = 23
    Anchors = [akLeft, akRight, akBottom]
    TabOrder = 1
    ExplicitTop = 357
    ExplicitWidth = 400
  end
  object btnSelect: TButton
    Left = 394
    Top = 359
    Width = 33
    Height = 25
    Anchors = [akRight, akBottom]
    Caption = '...'
    TabOrder = 2
    OnClick = btnSelectClick
    ExplicitLeft = 420
    ExplicitTop = 356
  end
  object btnPlay: TButton
    Left = 8
    Top = 405
    Width = 75
    Height = 25
    Anchors = [akLeft, akBottom]
    Caption = 'Play'
    TabOrder = 3
    OnClick = btnPlayClick
    ExplicitTop = 402
  end
  object btnStop: TButton
    Left = 96
    Top = 403
    Width = 75
    Height = 25
    Anchors = [akLeft, akBottom]
    Caption = 'Stop'
    TabOrder = 4
    OnClick = btnStopClick
    ExplicitTop = 400
  end
  object btnPrev: TButton
    Left = 184
    Top = 403
    Width = 40
    Height = 25
    Anchors = [akLeft, akBottom]
    Caption = '<'
    TabOrder = 5
    OnClick = btnPrevClick
    ExplicitTop = 400
  end
  object btnNext: TButton
    Left = 230
    Top = 403
    Width = 40
    Height = 25
    Anchors = [akLeft, akBottom]
    Caption = '>'
    TabOrder = 6
    OnClick = btnNextClick
    ExplicitTop = 400
  end
  object btnSavePic: TButton
    Left = 280
    Top = 403
    Width = 75
    Height = 25
    Anchors = [akLeft, akBottom]
    Caption = 'Save Pic'
    TabOrder = 7
    ExplicitTop = 400
  end
  object btnClose: TButton
    Left = 386
    Top = 402
    Width = 75
    Height = 25
    Anchors = [akRight, akBottom]
    Caption = 'Close'
    TabOrder = 8
    OnClick = btnCloseClick
    ExplicitLeft = 412
    ExplicitTop = 400
  end
  object pbFrames: TProgressBar
    Left = 32
    Top = 299
    Width = 409
    Height = 17
    Anchors = [akLeft, akRight, akBottom]
    TabOrder = 9
    ExplicitTop = 296
  end
  object dlgSelect: TOpenDialog
    Left = 384
    Top = 232
  end
  object tmrPlayback: TTimer
    Enabled = False
    Interval = 100
    OnTimer = tmrPlaybackTimer
    Left = 416
    Top = 232
  end
end
