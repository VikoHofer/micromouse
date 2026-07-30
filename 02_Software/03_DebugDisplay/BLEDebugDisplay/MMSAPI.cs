using System;
using System.Collections.Concurrent;

public class MMSAPI
{
    public static bool DEACTIVATEAPI = true;

    static int x = 0;
    static int y = 0;
    enum Direction
    {
        Top = 0,
        Left = 1,
        Right = 2,
        Bottom = 3,
    };
    static Direction MouseFacingDirection = Direction.Top;
    static ConcurrentQueue<Task> TaskQueue = new ConcurrentQueue<Task>();

    static bool IsTaskQueueThreadRunning = false;

    static void TryStartQueueThread()
    {
        if(IsTaskQueueThreadRunning) return;
        Task.Run(() => RunTaskQueue());
    }
    static async void RunTaskQueue()
    {
        if(IsTaskQueueThreadRunning) return;
        IsTaskQueueThreadRunning = true;

        do
        {
            IsTaskQueueThreadRunning = TaskQueue.TryDequeue(out Task? T) && T != null;
            if(IsTaskQueueThreadRunning && T != null)
            {
                T.Start();
                await T;
            }
        }
        while(IsTaskQueueThreadRunning);
    }

    static string GetResponse()
    {
        /*
        char[] buffer = new char[1024];

        int read = Console.In.Read(buffer, 0, buffer.Length);
        return new string(buffer, 0, read);*/
        return Console.ReadLine() ?? "";
    }

    public static int GetMazeWidth()
    {
        if(DEACTIVATEAPI) return -1;

        Console.WriteLine("mazeWidth");
        string? response = GetResponse();
        return response != null ? int.Parse(response) : -1;
    }

    public static int GetMazeHeight()
    {
        if(DEACTIVATEAPI) return -1;

        Console.WriteLine("mazeHeight");
        string? response = GetResponse();
        return response != null ? int.Parse(response) : -1;
    }

    public static bool GetWallFront()
    {
        if(DEACTIVATEAPI) return false;

        Console.WriteLine("wallFront");
        string? response = GetResponse();
        return response == "true";
    }

    public static bool GetWallRight()
    {
        if(DEACTIVATEAPI) return false;

        Console.WriteLine("wallRight");
        string? response = GetResponse();
        return response == "true";
    }

    public static bool GetWallLeft()
    {
        if(DEACTIVATEAPI) return false;

        Console.WriteLine("wallLeft");
        string? response = GetResponse();
        return response == "true";
    }

    public static void MoveForward(int distance = 1)
    {
        if(DEACTIVATEAPI) return;

        string cmd = "moveForward";
        if (distance != 1)
        {
            cmd += (" " + distance);
        }
        Console.WriteLine(cmd);

        /*string? response = GetResponse();
        if (response != "ack")
        {
            Console.Error.WriteLine(response);
            throw new Exception("Invalid response from simulator");
        }*/

        switch(MouseFacingDirection)
        {
            case Direction.Top:
                y += distance;
                break;
            case Direction.Bottom:
                y -= distance;
                break;
            case Direction.Right:
                x += distance;
                break;
            case Direction.Left:
                x -= distance;
                break;
        }
    }

    public static Task MoveForwardAsync(int distance = 1)
    {
        Task T = new Task(() => MoveForward(distance));
        TaskQueue.Enqueue(T);
        TryStartQueueThread();
        return T;
    }

    public static void TurnRight()
    {
        if(DEACTIVATEAPI) return;
        
        Console.WriteLine("turnRight");
        //string? ack = GetResponse();

        switch(MouseFacingDirection)
        {
            case Direction.Top:
                MouseFacingDirection = Direction.Right;
                break;
            case Direction.Left:
                MouseFacingDirection = Direction.Top;
                break;
            case Direction.Bottom:
                MouseFacingDirection = Direction.Left;
                break;
            case Direction.Right:
                MouseFacingDirection = Direction.Bottom;
                break;
        }
    }

    public static Task TurnRightAsync()
    {
        Task T = new Task(() => TurnRight());
        TaskQueue.Enqueue(T);
        TryStartQueueThread();
        return T;
    }

    public static void TurnLeft()
    {
        if(DEACTIVATEAPI) return;
        
        Console.WriteLine("turnLeft");
        //string? ack = GetResponse();
        
        switch(MouseFacingDirection)
        {
            case Direction.Top:
                MouseFacingDirection = Direction.Left;
                break;
            case Direction.Left:
                MouseFacingDirection = Direction.Bottom;
                break;
            case Direction.Bottom:
                MouseFacingDirection = Direction.Right;
                break;
            case Direction.Right:
                MouseFacingDirection = Direction.Top;
                break;
        }
    }

    public static Task TurnLeftAsync()
    {
        Task T = new Task(() => TurnLeft());
        TaskQueue.Enqueue(T);
        TryStartQueueThread();
        return T;
    }

    public static void SetWall(int x, int y, char direction)
    {
        if(DEACTIVATEAPI) return;
        
        Console.WriteLine($"setWall {x} {y} {direction}");
    }

    public static void SetWall(char direction) //direction: n, e, s, w
    {
        if(DEACTIVATEAPI) return;
        
        SetWall(GetCurrentX(), GetCurrentY(), direction);
    }

    public static Task SetWallAsync(char direction)
    {
        Task T = new Task(() => SetWall(direction));
        TaskQueue.Enqueue(T);
        TryStartQueueThread();
        return T;
    }

    public static void ClearWall(int x, int y, char direction)
    {
        if(DEACTIVATEAPI) return;
        
        Console.WriteLine($"clearWall {x} {y} {direction}");
    }

    public static void SetColor(int x, int y, char color)
    {
        if(DEACTIVATEAPI) return;
        
        Console.WriteLine($"setColor {x} {y} {color}");
    }

    public static void ClearColor(int x, int y)
    {
        if(DEACTIVATEAPI) return;
        
        Console.WriteLine($"clearColor {x} {y}");
    }

    public static void ClearAllColor()
    {
        if(DEACTIVATEAPI) return;
        
        Console.WriteLine("clearAllColor");
    }

    public static void SetText(int x, int y, string text)
    {
        if(DEACTIVATEAPI) return;
        
        Console.WriteLine($"setText {x} {y} {text}");
    }

    public static void ClearText(int x, int y)
    {
        if(DEACTIVATEAPI) return;
        
        Console.WriteLine($"clearText {x} {y}");
    }

    public static void ClearAllText()
    {
        if(DEACTIVATEAPI) return;
        
        Console.WriteLine("clearAllText");
    }

    public static bool WasReset()
    {
        if(DEACTIVATEAPI) return false;
        
        Console.WriteLine("wasReset");
        string? response = GetResponse();
        return response == "true";
    }

    public static void AckReset()
    {
        if(DEACTIVATEAPI) return;
        
        Console.WriteLine("ackReset");
        string? ack = GetResponse();
    }

    public static int GetCurrentX()
    {
        return x;
    }

    public static int GetCurrentY()
    {
        return y;
    }
}
