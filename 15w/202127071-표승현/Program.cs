using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Net;
using System.Net.Sockets;

namespace TCPServer
{
    class Program
    {
        const int SERVERPORT = 9000;
        const int BUFSIZE = 512;

        static void Main(string[] args)
        {
            int retval;

            Socket listen_sock = null;
            try
            {
                // 소켓 생성
                listen_sock = new Socket(AddressFamily.InterNetwork,
                    SocketType.Stream, ProtocolType.Tcp);

                // Bind()
                listen_sock.Bind(new IPEndPoint(IPAddress.Any, SERVERPORT));

                // Listen()
                listen_sock.Listen(Int32.MaxValue);
            }
            catch (Exception e)
            {
                Console.WriteLine(e.Message);
                Environment.Exit(1);
            }

            // 데이터 통신에 사용할 변수
            Socket client_sock = null;
            IPEndPoint clientaddr = null;
            byte[] buf = new byte[BUFSIZE];

            while (true)
            {
                try
                {
                    // accept()
                    client_sock = listen_sock.Accept();

                    // 접속한 클라이언트 정보 출력
                    clientaddr = (IPEndPoint)client_sock.RemoteEndPoint;
                    //Console.WriteLine("\n[TCP 서버] 클라이언트 접속: IP 주소={0}, 포트 번호={1}", clientaddr.Address, clientaddr.Port);

                    // 클라이언트와 데이터 통신
                    while (true)
                    {
                        // 데이터 받기
                        retval = client_sock.Receive(buf, BUFSIZE, SocketFlags.None);
                        if (retval == 0) break;

                        // 받은 데이터 출력
                        //Console.WriteLine("[TCP/{0}:{1}] {2}", clientaddr.Address, clientaddr.Port, Encoding.Default.GetString(buf, 0, retval));

                        // HTTP / 1.1 200 OK \r\n\r\n
                        // HTTP / 1.1 400 Bad Request \r\n\r\n
                        // HTTP / 1.1 404 Not Found \r\n\r\n

                        string data = Encoding.UTF8.GetString(buf, 0, retval); //받은 데이터를 문자열로 변환
                        string[] dataLine = data.Split('\n'); //문자열을 개행문자로 한줄씩 구분
                        string[] dataPart = dataLine[0].Trim().Split(' '); //첫번째 줄을 공백으로 구분(http 메서드, url, http 버전)

                        string httpMethod = dataPart[0]; //http 메서드
                        string url = dataPart[1]; //url
                        string httpVersion = dataPart[2]; //http 버전

                        string headers = string.Join("\n", dataLine.Skip(1).TakeWhile(line => !string.IsNullOrWhiteSpace(line))); //헤더 추출
                        string body = string.Join("\n", dataLine.SkipWhile(line => !string.IsNullOrWhiteSpace(line)).Skip(1)); //바디 추출

                        if (httpVersion != "HTTP/1.1") //추출한 버전이 http/1.1이 아니면
                        {
                            string response = $"{httpVersion} {(int)HttpStatusCode.BadRequest} Bad Request\r\n\r\n";
                            byte[] responseBytes = Encoding.UTF8.GetBytes(response);
                            client_sock.Send(responseBytes);
                            continue; // 다음 클라이언트 요청을 기다림
                        }

                        if (httpMethod == "GET" || httpMethod == "PUT" || httpMethod == "POST") 
                        {
                            if(url == "/" || url == "/test")
                            {
                                if(!string.IsNullOrEmpty(body)) //body가 있으면
                                {
                                    //Content-Type 정보 출력
                                    if (headers.Contains("Content-Type")) //headers 문자열에 "Content-Type"가 포함되어 있는지 확인
                                    {
                                        int contentTypeIndex = headers.IndexOf("Content-Type:"); //"Content-Type:"가 처음으로 나타나는 인덱스를 찾기
                                        int nextIndex = headers.IndexOf("\n", contentTypeIndex); //contentTypeIndex 이후에 나타나는 첫 번째 개행 문자의 인덱스
                                        string contentTypeLine = headers.Substring(contentTypeIndex, nextIndex - contentTypeIndex); //"Content-Type: "부터 개행 전까지
                                        Console.WriteLine(contentTypeLine);
                                    }
                                    Console.WriteLine("Content-Length: {0}", body.Length); //Content-Length 정보 출력

                                    string bodyText = body.Trim('{', '}').Trim().Replace("\"", "");
                                    string[] bodyLines = bodyText.Split(':');
                                    string bodyLine = bodyLines[1];
                                    System.IO.File.WriteAllText("..\\..\\log.txt", bodyLine); // body를 log 파일로 저장

                                    /*string bodyText = body.Trim('{', '}').Trim().Replace("\"", "");
                                    string[] bodyLines = bodyText.Split(':');
                                    string bodyLine = bodyLines[1];
                                    System.IO.File.WriteAllText("..\\..\\log.txt", bodyLine); // body를 log 파일로 저장*/

                                    /*string bodyText = body.Trim('{', '}').Trim().Replace("\"", "");
                                    string[] bodyLines = bodyText.Split(':');
                                    string bodyLine = "";
                                    for(int i=0; i<bodyLines.Length; i++)
                                    {
                                        bodyLine += bodyLines[i] + ":" + bodyLines[i + 1];
                                        i++;
                                    }
                                    System.IO.File.WriteAllText("..\\..\\log.txt", bodyLine); // body를 log 파일로 저장*/

                                    /*string Result = body.Trim('{', '}');
                                    Result = Result.Replace("\"", "").Trim();
                                    int bodyLength = Encoding.UTF8.GetByteCount(Result); // body 데이터의 바이트 수 계산
                                    string bodyPrint = Result + "\nlength:" + bodyLength;*/
                                    //System.IO.File.WriteAllText("..\\..\\log.txt", bodyPrint); // body를 log 파일로 저장

                                }
                                else
                                {
                                    Console.WriteLine(headers);
                                }

                                string response = $"{httpVersion} {(int)HttpStatusCode.OK} OK\r\n\r\n";
                                byte[] responseBytes = Encoding.UTF8.GetBytes(response);
                                client_sock.Send(responseBytes);
                                continue;
                            }
                            else
                            {
                                string response = $"{httpVersion} {(int)HttpStatusCode.NotFound} Not Found\r\n\r\n";
                                byte[] responseBytes = Encoding.Default.GetBytes(response);
                                client_sock.Send(responseBytes);
                                continue;
                            }
                        }
                        else
                        {
                            string response = $"{httpVersion} {(int)HttpStatusCode.BadRequest} Bad Request\r\n\r\n";
                            byte[] responseBytes = Encoding.Default.GetBytes(response);
                            client_sock.Send(responseBytes);
                            continue;
                        }
                    }

                    // 소켓 닫기
                    client_sock.Close();
                    //Console.WriteLine("[TCP 서버] 클라이언트 종료: IP 주소={0}, 포트 번호={1}", clientaddr.Address, clientaddr.Port);
                }
                catch (Exception e)
                {
                    Console.WriteLine(e.Message);
                    break;
                }
            }

            // 소켓 닫기
            listen_sock.Close();
        }
    }
}
