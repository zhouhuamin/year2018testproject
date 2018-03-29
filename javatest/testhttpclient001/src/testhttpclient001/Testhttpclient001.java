/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package testhttpclient001;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.net.HttpURLConnection;
import java.net.MalformedURLException;
import java.net.URL;

/**
 *
 * @author root
 */
public class Testhttpclient001 {

    /**
     * @param args the command line arguments
     */
    public static void main(String[] args) {
        // TODO code application logic here

        try {
            String destination = "http://www.baidu.com";
            // 定义Url
            URL url = new URL(destination);
            // 打开连接，强制转换为HttpUrlConnection
            HttpURLConnection conn = (HttpURLConnection) url.openConnection();
            conn.connect();
            // 获取输入流，并用BufferedReader进行封装
            BufferedReader reader = new BufferedReader(new InputStreamReader(
                    conn.getInputStream()));
            StringBuffer sBuffer = new StringBuffer();
            String line = "";
            // 读取返回流中的内容
            while ((line = reader.readLine()) != null) {
                sBuffer.append(line).append("\r\n");
            }
            // 打印状态码
            System.out.println("http response code============>" + conn.getResponseCode());
            // 打印返回的内容
            System.out.println("http response=========>" + sBuffer);
            conn.disconnect();
        } catch (MalformedURLException e) {
            e.printStackTrace();
        } catch (IOException e) {
            e.printStackTrace();
        }

    }

}
