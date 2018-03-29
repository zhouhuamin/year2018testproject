/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package testhttpclient002;

import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.OutputStreamWriter;
import java.net.HttpURLConnection;
import java.net.URL;

/**
 *
 * @author root
 */
public class Testhttpclient002 {

    /**
     * @param args the command line arguments
     */
    public static void main(String[] args) {
        // TODO code application logic here
        String path = "http://192.168.0.127:8080/httpserver001/HttpServer";
        String result = "";
        try {// 捕获跑出的IOException异常
            result = httpPost(path);
            System.out.println(result);
            result = httpGet(path);
            System.out.println(result);
        } catch (IOException e) {
            e.printStackTrace();
        }
    }

    /**
     * GET方法demo
     *
     * @param path
     * @return
     * @throws IOException
     */
    public static String httpGet(String path) throws IOException {
        URL url = new URL(path + "?param1=hjc1&param2=hjc2");
        HttpURLConnection conn = (HttpURLConnection) url.openConnection();
        conn.setRequestMethod("GET");
        conn.connect();
        BufferedReader reader = new BufferedReader(new InputStreamReader(
                conn.getInputStream()));
        StringBuffer sBuffer = new StringBuffer();
        String line = "";
        while ((line = reader.readLine()) != null) {
            sBuffer.append(line).append("\r\n");
        }
        reader.close();
        return sBuffer.toString();
    }

    /**
     * POST方法demo
     *
     * @param path
     * @return
     * @throws IOException
     */
    public static String httpPost(String path) throws IOException {
        URL url = new URL(path);
        HttpURLConnection conn = (HttpURLConnection) url.openConnection();
        conn.setRequestMethod("POST");
        // HttpURLConnection中的doInput默认为true，而doOutput默认为false，所以如果需要写内容到流，需要设置为true
        conn.setDoOutput(true);
        conn.connect();
        // Post方法不能通过url传递参数，需通过将参数写入body体来传递
        BufferedWriter writer = new BufferedWriter(new OutputStreamWriter(
                conn.getOutputStream()));
        writer.write("param1=hjc1&param2=hjc2");
        writer.close();
        BufferedReader reader = new BufferedReader(new InputStreamReader(
                conn.getInputStream()));
        StringBuffer sBuffer = new StringBuffer();
        String line = "";
        while ((line = reader.readLine()) != null) {
            sBuffer.append(line).append("\r\n");
        }
        reader.close();
        return sBuffer.toString();
    }
}
